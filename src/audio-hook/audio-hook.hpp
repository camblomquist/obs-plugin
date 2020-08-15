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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <audioclient.h>

#include "audio-hook-info.hpp"
#include "funchook.hpp"

typedef HRESULT(STDMETHODCALLTYPE *get_buffer_t)(IAudioRenderClient *, UINT32,
						 BYTE **);
typedef HRESULT(STDMETHODCALLTYPE *release_buffer_t)(IAudioRenderClient *,
						     UINT32, DWORD);

struct CaptureData {
	IAudioRenderClient *client;

	uint32_t samplesPerSec;
	uint32_t blockAlign;

	uint32_t framesWritten;

	uint8_t *leech;
	
	uint8_t *buffer;
	uint32_t capacity;
};

struct CaptureThread {
	CRITICAL_SECTION mutexAudio;
	CRITICAL_SECTION mutexData;

	uint8_t *buffer;

	HANDLE copyThread;
	HANDLE signalCopy;
	HANDLE signalStop;
};

class Capture {
	DWORD pid;
	uint32_t idCounter = 0;

	HANDLE signalRestart;
	HANDLE signalStop;
	HANDLE signalReady;
	HANDLE signalExit;
	HANDLE signalRecv;
	HANDLE signalInit;
	HANDLE mutexAudio;

	bool active = false;
	bool hooked = false;

	HookInfo *hookInfo;
	SharedMemoryData *sharedMemoryData;

	CaptureData data = {};
	CaptureThread thread = {};

	HANDLE hookThread = nullptr;

	FunctionHook<get_buffer_t> getBufferHook;
	FunctionHook<release_buffer_t> releaseBufferHook;

	Capture();

	static Capture &Instance();

#pragma region Signaling
	static bool IsAlive();
	static bool IsActive();
	static bool IsStopped();
	static bool HasRestarted();

	static bool ShouldStop();
	static bool ShouldInitialize();

	static bool SignalRestart();
	static bool SignalReady();
	static bool SignalReceive();

	static DWORD WaitForInitSignal();

	static void SetActive(bool);
#pragma endregion

#pragma region Hooking
	static HookInfo &HookInfo();

	static bool IsHookable();
	static bool IsHooked();
	static void Hook();

	static DWORD WINAPI HookThread(HANDLE);

	static HRESULT STDMETHODCALLTYPE HookGetBuffer(IAudioRenderClient *,
						       UINT32, BYTE **);

	static HRESULT STDMETHODCALLTYPE HookReleaseBuffer(IAudioRenderClient *,
							   UINT32, DWORD);
#pragma endregion

#pragma region Capturing
	static bool InitializeCapture();
	static bool InitializeSharedMemory(size_t);
	static bool InitializeCaptureThread();

	static void CaptureAudioData();
#pragma endregion

public:
	static void Start();

	Capture(Capture const &) = delete;
	void operator=(Capture const &) = delete;
};