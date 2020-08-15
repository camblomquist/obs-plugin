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

#include "audio-hook.hpp"

#include <windows.h>

#include "hook-helper.hpp"
#include "audio-hook-ver.hpp"

#pragma region Miscellany
#ifndef ALIGN
#define ALIGN(bytes, align) (((bytes) + ((align)-1)) & ~((align)-1))
#endif
#pragma endregion

#pragma region Capture Implementation
Capture::Capture()
{
	pid = GetCurrentProcessId();

	signalRestart = CreateEventWithId(EVENT_CAPTURE_RESTART, pid);
	if (!signalRestart)
		throw "Could not create restart signal";

	signalStop = CreateEventWithId(EVENT_CAPTURE_STOP, pid);
	if (!signalStop)
		throw "Could not create stop signal";

	signalReady = CreateEventWithId(EVENT_HOOK_READY, pid);
	if (!signalRestart)
		throw "Could not create ready signal";

	signalExit = CreateEventWithId(EVENT_HOOK_EXIT, pid);
	if (!signalExit)
		throw "Could not create exit signal";

	signalRecv = CreateEventWithId(EVENT_CAPTURE_RECEIVE, pid);
	if (!signalRecv)
		throw "Could not create receive signal";

	signalInit = CreateEventWithId(EVENT_HOOK_INIT, pid);
	if (!signalInit)
		throw "Could not create init signal";

	mutexAudio = CreateMutexWithId(MUTEX_AUDIO, pid);
	if (!mutexAudio)
		throw "Could not create audio mutex";

	HANDLE mapping = CreateHookInfo(pid);
	if (!mapping)
		throw "Could not create hook info file map";

	hookInfo = reinterpret_cast<struct HookInfo *>(MapViewOfFile(
		mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct HookInfo)));
	if (!hookInfo)
		throw "Failed to map hook info";
	CloseHandle(mapping);
}

Capture &Capture::Instance()
{
	static Capture instance;

	return instance;
}

void Capture::Start()
{
	HANDLE dllMainThread;
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
			     GetCurrentProcess(), &dllMainThread, SYNCHRONIZE,
			     false, 0)) {
		throw "Failed to get current thread handle";
	}

	Instance().hookThread =
		CreateThread(nullptr, 0, HookThread, dllMainThread, 0, 0);
	if (!Instance().hookThread) {
		throw "Failed to create capture thread";
	}
}

#pragma region Signaling
bool Capture::IsAlive()
{
	return true;
}

bool Capture::IsActive()
{
	return Instance().active;
}

bool Capture::IsStopped()
{
	return Signaled(Instance().signalStop);
}

bool Capture::HasRestarted()
{
	return Signaled(Instance().signalRestart);
}

bool Capture::ShouldStop()
{
	return false;
}

bool Capture::ShouldInitialize()
{
	return !IsActive() && HasRestarted() && IsAlive();
}

bool Capture::SignalRestart()
{
	return SetEvent(Instance().signalRestart);
}

bool Capture::SignalReady()
{
	return SetEvent(Instance().signalReady);
}

bool Capture::SignalReceive()
{
	return SetEvent(Instance().signalRecv);
}

DWORD Capture::WaitForInitSignal()
{
	return WaitForSingleObject(Instance().signalInit, INFINITE);
}

void Capture::SetActive(bool b)
{
	Instance().active = b;
}
#pragma endregion

#pragma region Hooking
HookInfo &Capture::HookInfo()
{
	return *Instance().hookInfo;
}

bool Capture::IsHookable()
{
	return HookInfo().offsets.getBuffer != 0 &&
	       HookInfo().offsets.releaseBuffer != 0;
}

bool Capture::IsHooked()
{
	return Instance().hooked;
}

void Capture::Hook()
{
	if (IsHooked() || !IsHookable()) {
		return;
	}

	// Probably likely to be valid for the lifetime of the process
	HMODULE module = GetModuleHandleW(L"audioses.dll");
	if (!module) {
		// LOG HERE
		return;
	}

	uintptr_t base = reinterpret_cast<uintptr_t>(module);
	get_buffer_t getBuffer = reinterpret_cast<get_buffer_t>(
		base + HookInfo().offsets.getBuffer);
	release_buffer_t releaseBuffer = reinterpret_cast<release_buffer_t>(
		base + HookInfo().offsets.releaseBuffer);

	Instance().getBufferHook = FunctionHook<get_buffer_t>(
		getBuffer, HookGetBuffer, "IAudioRenderClient::GetBuffer");
	Instance().releaseBufferHook = FunctionHook<release_buffer_t>(
		releaseBuffer, HookReleaseBuffer,
		"IAudioRenderClient::ReleaseBuffer");

	// LOG HERE
}

DWORD WINAPI Capture::HookThread(HANDLE dllMainThread)
{
	// Wait for DllMain to finish
	if (dllMainThread) {
		WaitForSingleObject(dllMainThread, 100);
		CloseHandle(dllMainThread);
	}

	SignalRestart();

	WaitForInitSignal();

	while (!IsHooked()) {
		Hook();
		Sleep(40);
	}

	// From Jim:
	/* this causes it to check every 4 seconds, but still with
	 * a small sleep interval in case the thread needs to stop */
	for (size_t n = 0;; n++) {
		if (n % 100 == 0)
			Hook();
		Sleep(40);
	}

	return 0;
}

HRESULT STDMETHODCALLTYPE Capture::HookGetBuffer(IAudioRenderClient *client,
						 UINT32 framesRequested,
						 BYTE **ppData)
{
	HRESULT hr;
	CaptureData &data = Instance().data;
	FunctionHook<get_buffer_t> &getBufferHook = Instance().getBufferHook;

	getBufferHook.Unhook();
	hr = getBufferHook.Call()(client, framesRequested, ppData);
	getBufferHook.Rehook();

	if (!data.client && !IsActive()) {
		data.client = client;
	}

	if (client == data.client) {
		data.leech = *ppData;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE Capture::HookReleaseBuffer(IAudioRenderClient *client,
						     UINT32 framesWritten,
						     DWORD dwFlags)
{
	HRESULT hr;
	CaptureData &data = Instance().data;
	FunctionHook<release_buffer_t> &releaseBufferHook =
		Instance().releaseBufferHook;

	if (client == data.client) {
		if (ShouldInitialize()) {
			InitializeCapture();
		}
		if (IsActive()) {
			uint32_t size = framesWritten * data.blockAlign;
			if (size > data.capacity) {
				// This should be safe if not previously allocated
				delete[] data.buffer;
				data.buffer = new uint8_t[size];
			}

			// Hello darkness, my old friend...
			if (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT) {
				memset(&data.buffer, 0, size);
			} else {
				memcpy(&data.buffer, &data.leech, size);
			}

			CaptureAudioData();
		}
	}

	releaseBufferHook.Unhook();
	hr = releaseBufferHook.Call()(client, framesWritten, dwFlags);
	releaseBufferHook.Rehook();

	return hr;
}

#pragma endregion

#pragma region Capturing
bool Capture::InitializeCapture() {
	CaptureData &cData = Instance().data;

	// No easy (possible?) way of getting this info local
	cData.samplesPerSec = HookInfo().samplesPerSec;
	cData.blockAlign = HookInfo().blockAlign;

	/* Without access to the AudioClient that created the AudioRenderClient,
	 * we have no way of knowing the actual size of the allocated buffer.
	 * In exclusive mode, this is the maximum capacity of a push buffer.
	 * In shared mode, we hope it's less */
	uint32_t size = cData.samplesPerSec * cData.blockAlign * 2;
	uint32_t headerAligned = ALIGN(sizeof(SharedMemoryData), 32);
	uint32_t sizeAligned = ALIGN(size, 32);
	size_t totalSize = static_cast<size_t>(headerAligned) + sizeAligned + 32;

	if (!InitializeSharedMemory(totalSize)) {
		// LOG HERE
		return false;
	}

	SharedMemoryData *&smData = Instance().sharedMemoryData;

	uintptr_t pos, ptr;
	pos = ptr = reinterpret_cast<uintptr_t>(smData);
	pos += headerAligned;
	pos &= ~(32 - 1);
	pos -= ptr;

	if (pos < sizeof(SharedMemoryData))
		pos += 32;

	smData->buffer = pos;

	HookInfo().versionMajor = HOOK_VERSION_MAJOR;
	HookInfo().versionMinor = HOOK_VERSION_MINOR;
	HookInfo().mapId = Instance().idCounter;
	HookInfo().mapSize = totalSize;

	if (!InitializeCaptureThread()) {
		// LOG HERE?
		return false;
	}

	if (!SignalReady()) {
		// LOG HERE
		return false;
	}

	SetActive(true);
	return true;
}

bool Capture::InitializeSharedMemory(size_t size) 
{
	wchar_t name[64];
	swprintf(name, 64, SHMEM_BUFFER L"_%u_%u", Instance().pid,
		 ++Instance().idCounter);

	HANDLE mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr,
					    PAGE_READWRITE, 0, size, name);
	if (!mapping) {
		// LOG HERE
		return false;
	}

	SharedMemoryData *&smData = Instance().sharedMemoryData;
	smData = reinterpret_cast<SharedMemoryData *>(
		MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, size));
	if (!smData) {
		// LOG HERE
		CloseHandle(mapping);
		return false;
	}

	CloseHandle(mapping);
	return true;
}

bool Capture::InitializeCaptureThread() {

	return true;
}
#pragma endregion
#pragma endregion