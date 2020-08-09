/*
Windows Audio Session Capture Plugin for OBS Studio
Copyright (C) 2020 Cameron Blomquist cameron@blomqu.ist

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <util/dstr.hpp>

#include "plugin-macros.generated.h"
#include "helpers/audio-session-helper.hpp"

#pragma region Macros
/* clang-format off */
#define SETTING_SESSION				"session"
#define SETTING_ANTI_CHEAT_HOOK		"anti_cheat_hook"
#define SETTING_HOOK_RATE			"hook_rate"

#define TEXT_AUDIO_CAPTURE			obs_module_text("AudioCapture")
#define TEXT_SESSION				obs_module_text("AudioCapture.Session")
#define TEXT_ANTI_CHEAT_HOOK		obs_module_text("AudioCapture.AntiCheatHook")
#define TEXT_HOOK_RATE				obs_module_text("AudioCapture.HookRate")
#define TEXT_HOOK_RATE_SLOW			obs_module_text("AudioCapture.HookRate.Slow")
#define TEXT_HOOK_RATE_NORMAL		obs_module_text("AudioCapture.HookRate.Normal")
#define TEXT_HOOK_RATE_FAST			obs_module_text("AudioCapture.HookRate.Fast")
#define TEXT_HOOK_RATE_FASTEST		obs_module_text("AudioCapture.HookRate.Fastest")
/* clang-format on */

#define OBS_KSAUDIO_SPEAKER_4POINT1 \
	(KSAUDIO_SPEAKER_SURROUND | SPEAKER_LOW_FREQUENCY)
#pragma endregion

/* Fuck C++ for having literally the worst implementation of enumerated
 * types in any language I've ever used */
enum class HookRate { SLOW, NORMAL, FAST, FASTEST }; 

#pragma region Class Definition
class AudioCaptureSource {
	obs_source_t *source;

	std::string session;
	std::string session_id;
	std::string device_id;

	speaker_layout speakers;
	audio_format format;
	uint32_t samples_per_sec;

	bool anticheat_hook;
	HookRate hook_rate;

	void Start();
	void Stop();

public:
	AudioCaptureSource(obs_data_t *settings, obs_source_t *source);
	~AudioCaptureSource();

	void Update(obs_data_t *settings);
};
#pragma endregion

#pragma region Misc Utility Functions
// Borrowed from win-wasapi (win-wasapi.cpp)
static speaker_layout ConvertSpeakerLayout(DWORD layout, WORD channels)
{
	switch (layout) {
	case KSAUDIO_SPEAKER_2POINT1:
		return SPEAKERS_2POINT1;
	case KSAUDIO_SPEAKER_SURROUND:
		return SPEAKERS_4POINT0;
	case OBS_KSAUDIO_SPEAKER_4POINT1:
		return SPEAKERS_4POINT1;
	case KSAUDIO_SPEAKER_5POINT1_SURROUND:
		return SPEAKERS_5POINT1;
	case KSAUDIO_SPEAKER_7POINT1_SURROUND:
		return SPEAKERS_7POINT1;
	}

	return static_cast<speaker_layout>(channels);
}
#pragma endregion

#pragma region Class Implementation
#pragma region Public
AudioCaptureSource::AudioCaptureSource(obs_data_t *settings,
				       obs_source_t *source)
	: source(source)
{
	Update(settings);
}

AudioCaptureSource::~AudioCaptureSource()
{
	Stop();
}

void AudioCaptureSource::Update(obs_data_t *settings)
{
	std::string new_session =
		obs_data_get_string(settings, SETTING_SESSION);
	bool reset = new_session != session;

	if (reset) {
		Stop();
	}

	session = new_session;
	if (session.empty()) {
		session_id = "";
		device_id = "";
	} else {
		size_t delim = new_session.find("::");
		session_id = session.substr(0, delim);
		device_id = session.substr(delim);
	}	

	anticheat_hook = obs_data_get_bool(settings, SETTING_ANTI_CHEAT_HOOK);
	hook_rate = static_cast<HookRate>(
		obs_data_get_int(settings, SETTING_HOOK_RATE));

	if (reset) {
		Start();
	}
}
#pragma endregion

#pragma region Private
void AudioCaptureSource::Start() {}

void AudioCaptureSource::Stop() {}
#pragma endregion
#pragma endregion

#pragma region OBS Source Info
static const char *GetAudioCaptureSourceName(void *)
{
	return TEXT_AUDIO_CAPTURE;
}

static void *CreateAudioCaptureSource(obs_data_t *settings,
				      obs_source_t *source)
{
	return new AudioCaptureSource(settings, source);
}

static void DestroyAudioCaptureSource(void *data)
{
	delete static_cast<AudioCaptureSource *>(data);
}

static void UpdateAudioCaptureSource(void *data, obs_data_t *settings)
{
	static_cast<AudioCaptureSource *>(data)->Update(settings);
}

static void GetAudioCaptureSourceDefaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, SETTING_ANTI_CHEAT_HOOK, true);
	obs_data_set_default_int(settings, SETTING_HOOK_RATE, static_cast<int>(HookRate::NORMAL));
}

static obs_properties_t *GetAudioCaptureSourceProperties(void *data)
{
	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;

	p = obs_properties_add_list(props, SETTING_SESSION, TEXT_SESSION,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, "", "");

	std::vector<AudioSessionInfo> sessions = GetAudioSessions();

	for (auto session : sessions) {
		DStr desc;
		DStr id;

		if (session.session_name.empty()) {
			session.session_name = "Unnamed Session";
		}

		dstr_printf(desc, "[%s]: %s (%s)", session.exe.c_str(),
			    session.session_name.c_str(),
			    session.device_name.c_str());
		dstr_printf(id, "%s::%s", session.device_id.c_str(),
			    session.session_id.c_str());
		obs_property_list_add_string(p, desc, id);
	}

	p = obs_properties_add_bool(props, SETTING_ANTI_CHEAT_HOOK,
				    TEXT_ANTI_CHEAT_HOOK);

	p = obs_properties_add_list(props, SETTING_HOOK_RATE, TEXT_HOOK_RATE,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, TEXT_HOOK_RATE_SLOW,
				  static_cast<int>(HookRate::SLOW));
	obs_property_list_add_int(p, TEXT_HOOK_RATE_NORMAL,
				  static_cast<int>(HookRate::NORMAL));
	obs_property_list_add_int(p, TEXT_HOOK_RATE_FAST,
				  static_cast<int>(HookRate::FAST));
	obs_property_list_add_int(p, TEXT_HOOK_RATE_FASTEST,
				  static_cast<int>(HookRate::FASTEST));

	return props;
}

void RegisterAudioCaptureSource()
{
	/* Did you know that designated initializers have been in C since C99
	 * But weren't in C++ until C++20? I learned that just now. */
	obs_source_info info = {};
	info.id = "audio_session_capture";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE |
			    OBS_SOURCE_DO_NOT_SELF_MONITOR;
	info.get_name = GetAudioCaptureSourceName;
	info.create = CreateAudioCaptureSource;
	info.destroy = DestroyAudioCaptureSource;
	info.update = UpdateAudioCaptureSource;
	info.get_defaults = GetAudioCaptureSourceDefaults;
	info.get_properties = GetAudioCaptureSourceProperties;
	info.icon_type = OBS_ICON_TYPE_AUDIO_OUTPUT;

	obs_register_source(&info);
}
#pragma endregion