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

#include "get-audio-offsets.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cinttypes>
#include <cstdio>

#include "audio-hook-info.hpp"

int main()
{
	SetErrorMode(SEM_FAILCRITICALERRORS);

	AudioRenderClientOffsets offsets = GetOffsets();

	// Formatted printing with cout is miserable, printf can stay
	printf("[IAudioRenderClient]\n");
	printf("getBuffer=0x%" PRIx64 "\n", offsets.getBuffer);
	printf("releaseBuffer=0x%" PRIx64 "\n", offsets.releaseBuffer);

	return 0;
}
