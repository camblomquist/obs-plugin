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

#pragma once

#include <obs-module.h>

#include <string>

/* Fuck C++ for having literally the worst implementation of enumerated
 * types in any language I've ever used */
enum class HookRate { SLOW, NORMAL, FAST, FASTEST };

class AudioCaptureSource {
	obs_source_t *source;

	std::string session;
	std::string sessionId;
	std::string deviceId;

	speaker_layout speakers;
	audio_format format;
	uint32_t samplesPerSec;

	bool anticheatHook;
	HookRate hookRate;

	void Start();
	void Stop();

public:

	AudioCaptureSource(obs_data_t *settings, obs_source_t *source);
	~AudioCaptureSource();

	void Update(obs_data_t *settings);
};

void RegisterAudioCaptureSource();