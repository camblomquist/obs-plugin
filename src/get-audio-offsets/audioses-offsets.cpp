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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "get-audio-offsets.hpp"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

struct Info {
	bool initialized;

	HMODULE module;
	IMMDeviceEnumerator *enumerator;
	IMMDevice *device;
	IAudioClient *client;
	WAVEFORMATEX *format;
	IAudioRenderClient *render;
};

static bool Initialize(Info &info)
{
	HRESULT hr;

	hr = CoInitialize(nullptr);
	if (FAILED(hr))
		return false;
	info.initialized = true;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
			      IID_IMMDeviceEnumerator,
			      (void **)&info.enumerator);
	if (FAILED(hr))
		return false;

	hr = info.enumerator->GetDefaultAudioEndpoint(eRender, eConsole,
						      &info.device);
	if (FAILED(hr))
		return false;

	hr = info.device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr,
				   (void **)&info.client);
	if (FAILED(hr))
		return false;

	hr = info.client->GetMixFormat(&info.format);
	if (FAILED(hr))
		return false;

	hr = info.client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 10000, 0,
				 info.format, nullptr);
	if (FAILED(hr))
		return false;

	hr = info.client->GetService(IID_IAudioRenderClient, (void **)&info.render);
	if (FAILED(hr))
		return false;

	info.module = GetModuleHandleA("audioses.dll");
	if (!info.module)
		return false;

	return true;
}

static void Free(Info &info)
{
	if (info.render)
		info.render->Release();
	if (info.format)
		CoTaskMemFree(info.format);
	if (info.client)
		info.client->Release();
	if (info.device)
		info.device->Release();
	if (info.enumerator)
		info.enumerator->Release();
	if (info.initialized)
		CoUninitialize();
}

AudioRenderClientOffsets GetOffsets()
{
	Info info = {};
	AudioRenderClientOffsets offsets = {};

	if (Initialize(info)) {
		offsets.getBuffer = VTableOffset(info.module, info.render, 3);
		offsets.releaseBuffer = VTableOffset(info.module, info.render, 4);
	}

	Free(info);

	return offsets;
}
