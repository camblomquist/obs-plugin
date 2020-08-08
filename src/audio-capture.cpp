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

#include "helpers/audio-session-helper.hpp"

#pragma region Macros
/* clang-format off */
#define SETTING_SESSION				"session"

#define TEXT_AUDIO_CAPTURE			obs_module_text("AudioCapture")
#define TEXT_SESSION				obs_module_text("AudioCapture.Session")
/* clang-format on */
#pragma endregion

#pragma region Class Definition
class AudioCaptureSource {
	obs_source_t *source;

public:
	AudioCaptureSource(obs_data_t *settings, obs_source_t *source);
	~AudioCaptureSource();

	void Update(obs_data_t *settings);
};
#pragma endregion

#pragma region Class Implementation
AudioCaptureSource::AudioCaptureSource(obs_data_t *settings,
				       obs_source_t *source)
	: source(source)
{
}

AudioCaptureSource::~AudioCaptureSource() {}

void AudioCaptureSource::Update(obs_data_t *settings) {}
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

static void GetAudioCaptureSourceDefaults(obs_data_t *settings) {}

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
		dstr_printf(desc, "[%s]: %s (%s)", session.exe.c_str(),
			    session.session_name.c_str(),
			    session.device_name.c_str());
		obs_property_list_add_string(p, desc,
					     session.session_id.c_str());
	}

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