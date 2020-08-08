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

#include "windows-helper.hpp"

#include <util/platform.h>
#include <util/windows/WinHandle.hpp>

#include <psapi.h>

#include <string>

// Borrowed from win-wasapi (enum-wasapi.cpp), in function form
std::string StringFromLPWSTR(LPWSTR str)
{
	std::string result;
	size_t len_src = wcslen(str);
	size_t len_dest = os_wcs_to_utf8(str, len_src, nullptr, 0) + 1;
	result.resize(len_dest);
	os_wcs_to_utf8(str, len_src, &result[0], len_dest);

	return result;
}

std::string GetProcessExeName(DWORD pid)
{
	std::string name;
	WCHAR wname[MAX_PATH];
	WinHandle process =
		OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
	if (process.Valid() &&
	    GetProcessImageFileNameW(process, wname, MAX_PATH)) {
		name = StringFromLPWSTR(wname);
		size_t slash = name.rfind('\\');
		if (slash != std::string::npos) {
			name = name.substr(slash + 1);
		}
	}

	return name;
}