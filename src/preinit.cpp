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

#include "preinit.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <obs-module.h>
#include <util/config-file.h>
#include <util/windows/WinHandle.hpp>

#include <string>

#include "plugin-macros.hpp"
#include "audio-capture.hpp"
#include "audio-hook/audio-hook-info.hpp"
#include "helpers/process-pipe.hpp"

static WinHandle preinitThread;

static AudioRenderClientOffsets LoadOffsets(bool is32bit)
{
	AudioRenderClientOffsets offsets = {};

	std::string data;
	char rawData[2048];

	std::string exe = std::string("get-audio-offsets")
				  .append(is32bit ? "32" : "64")
				  .append(".exe");
	char *exe_path = obs_module_file(exe.c_str());

	try {
		ProcessPipe pipe(exe_path);
		while (size_t len = pipe.Read(rawData, sizeof(rawData))) {
			data.append(rawData, len);
		}
	} catch (DWORD errorCode) {
		bwarn("'%s' Failed: %lu", exe_path, errorCode);
		goto end;
	}

	if (data.empty()) {
		bwarn("Failed to read from '%s'", exe_path);
		goto end;
	}

	config_data *config;
	if (config_open_string(&config, data.c_str()) != CONFIG_SUCCESS) {
		bwarn("Failed to configure");
		goto end;
	}

	offsets.getBuffer = static_cast<uint32_t>(
		config_get_uint(config, "IAudioRenderClient", "getBuffer"));
	offsets.releaseBuffer = static_cast<uint32_t>(
		config_get_uint(config, "IAudioRenderClient", "releaseBuffer"));

	config_close(config);

end:
	bfree(exe_path);
	return offsets;
}

static DWORD PreinitThread(LPVOID)
{
	AudioCaptureSource::offsets32 = LoadOffsets(true);
	AudioCaptureSource::offsets64 = LoadOffsets(false);
	return 0;
}

void Preinitialize()
{
	if (!preinitThread.Valid()) {
		preinitThread = CreateThread(nullptr, 0, PreinitThread, nullptr,
					     0, nullptr);

		if (!preinitThread.Valid()) {
			bwarn("Failed to Preinitialize: %lu", GetLastError());
		}
	}
}

void WaitForPreinitialization()
{
	static bool initialized = false;

	if (!initialized) {
		if (preinitThread.Valid()) {
			WaitForSingleObject(preinitThread, INFINITE);
			preinitThread = nullptr;
		}
		initialized = true;
	}
}