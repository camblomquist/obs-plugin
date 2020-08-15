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

#include <handleapi.h>

#include <cstdint>
#include <cwchar>

/* clang-format off */

#define EVENT_CAPTURE_RESTART	L"AudioCaptureHook_Restart"
#define EVENT_CAPTURE_STOP		L"AudioCaptureHook_Stop"
#define EVENT_CAPTURE_RECEIVE	L"AudioCaptureHook_DataReceived"

#define EVENT_HOOK_READY		L"AudioCaptureHook_HookReady"
#define EVENT_HOOK_EXIT			L"AudioCaptureHook_Exit"

#define EVENT_HOOK_INIT			L"AudioCaptureHook_Initialize"

#define MUTEX_AUDIO				L"AudioCaptureHook_AudioMutex"

#define SHMEM_HOOK_INFO			L"AudioCaptureHook_HookInfo"
#define SHMEM_BUFFER			L"AudioCaptureHook_Buffer"

#define LOG_PIPE				L"AudioCaptureHook_LogPipe"

/* clang-format on */

// Not sure how necessary this is
#pragma pack(push, 8)

struct AudioRenderClientOffsets {
	uintptr_t getBuffer;
	uintptr_t releaseBuffer;
};

struct SharedMemoryData {
	uintptr_t buffer;
	volatile size_t size;
};

struct HookInfo {
	uint32_t versionMajor;
	uint32_t versionMinor;

	uint32_t channels;
	uint32_t samplesPerSec;
	uint32_t blockAlign;

	uint32_t mapId;
	uint32_t mapSize;

	AudioRenderClientOffsets offsets;
};
#pragma pack(pop)

static inline HANDLE CreateHookInfo(DWORD id) {
	HANDLE handle = nullptr;
	
	wchar_t newName[64];
	const int len = swprintf(newName, 64, SHMEM_HOOK_INFO L"%lu", id);
	if (len > 0) {
		handle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
					    PAGE_READWRITE, 0, sizeof(HookInfo),
					    newName);
	}

	return handle;
}