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

#include "audio-session-helper.hpp"

#include <util/base.h>
// Why did Jim create a custom com pointer class? The world is indeed ending.
#include <util/windows/ComPtr.hpp>
#include <util/windows/CoTaskMemPtr.hpp>
#include <util/windows/HRError.hpp>

#include "../plugin-macros.generated.h"
#include "windows-helper.hpp"

#define AUDCLNT_S_NO_SINGLE_PROCESS AUDCLNT_SUCCESS(0x00d)

// Borrowed from win-wasapi (win-wasapi.cpp)
std::string GetDeviceName(IMMDevice *device)
{
	std::string device_name;
	ComPtr<IPropertyStore> store;
	HRESULT hr;

	if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store))) {
		PROPVARIANT nameVar;

		PropVariantInit(&nameVar);
		hr = store->GetValue(PKEY_Device_FriendlyName, &nameVar);

		if (SUCCEEDED(hr) && nameVar.pwszVal && *nameVar.pwszVal) {
			device_name = StringFromLPWSTR(nameVar.pwszVal);
		}
	}

	return device_name;
}

static std::vector<AudioSessionInfo> GetAudioSessionsInternal()
{
	HRESULT hr;
	std::vector<AudioSessionInfo> res_sessions;
	ComPtr<IMMDeviceEnumerator> enumerator;
	ComPtr<IMMDeviceCollection> collection;
	UINT device_count;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			      IID_PPV_ARGS(&enumerator));
	// I miss Rust Results
	if (FAILED(hr))
		throw HRError("Failed to create enumerator", hr);

	hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE,
					    &collection);
	if (FAILED(hr))
		throw HRError("Failed to enumerate devices", hr);

	hr = collection->GetCount(&device_count);
	if (FAILED(hr))
		throw HRError("Failed to get device count", hr);

	for (UINT i = 0; i < device_count; i++) {
		ComPtr<IMMDevice> device;
		ComPtr<IAudioSessionManager2> manager;
		ComPtr<IAudioSessionEnumerator> sessions;
		CoTaskMemPtr<WCHAR> w_device_id;
		std::string device_name;
		std::string device_id;
		int session_count;

		hr = collection->Item(i, &device);
		if (FAILED(hr))
			continue;

		hr = device->GetId(&w_device_id);
		if (FAILED(hr) || !w_device_id || !*w_device_id)
			continue;

		device_name = GetDeviceName(device);
		device_id = StringFromLPWSTR(w_device_id);

		hr = device->Activate(__uuidof(IAudioSessionManager2),
				      CLSCTX_ALL, NULL,
				      reinterpret_cast<void **>(&manager));
		if (FAILED(hr))
			continue;

		hr = manager->GetSessionEnumerator(&sessions);
		if (FAILED(hr))
			continue;

		hr = sessions->GetCount(&session_count);
		if (FAILED(hr))
			continue;

		for (int j = 0; j < session_count; j++) {
			ComPtr<IAudioSessionControl> session1;
			CoTaskMemPtr<WCHAR> w_session_name;
			CoTaskMemPtr<WCHAR> w_session_id;
			DWORD process_id;
			AudioSessionInfo info;

			hr = sessions->GetSession(j, &session1);
			if (FAILED(hr))
				continue;
			// ATL has a default constructor for their CComQIPtr
			// Why can't you do that, Jim?
			ComQIPtr<IAudioSessionControl2> session2(session1);

			if (session2->IsSystemSoundsSession() == S_OK)
				continue;

			hr = session2->GetDisplayName(&w_session_name);
			if (FAILED(hr))
				continue;

			hr = session2->GetSessionIdentifier(&w_session_id);
			if (FAILED(hr))
				continue;

			hr = session2->GetProcessId(&process_id);
			if (FAILED(hr))
				continue;

			info.session_name = StringFromLPWSTR(w_session_name);
			info.session_id = StringFromLPWSTR(w_session_id);
			info.device_name = device_name;
			info.device_id = device_id;
			info.process_id = process_id;
			info.exe = GetProcessExeName(process_id);

			res_sessions.push_back(info);
		}
	}

	return res_sessions;
}

std::vector<AudioSessionInfo> GetAudioSessions()
{
	std::vector<AudioSessionInfo> sessions;

	// I *really* miss Rust Results
	try {
		sessions = GetAudioSessionsInternal();
	} catch (HRError &error) {
		bwarn("%s: %lX", error.str, error.hr);
	}

	return sessions;
}