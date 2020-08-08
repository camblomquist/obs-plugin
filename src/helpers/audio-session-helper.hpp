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

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>

#include <vector>
#include <string>

struct AudioSessionInfo {
	std::string session_name;
	std::string session_id;

	std::string device_name;
	std::string device_id;

	DWORD process_id;
	std::string exe;
};

std::string GetDeviceName(IMMDevice *device);

std::vector<AudioSessionInfo> GetAudioSessions();