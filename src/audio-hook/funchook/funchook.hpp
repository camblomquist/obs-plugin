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

#include <audioclient.h>

#include "funchook.h"

typedef HRESULT(STDMETHODCALLTYPE *dummyFunction)(IAudioRenderClient *);

// Whether or not this is good, it is convenient
template<class T> class FunctionHook {
	func_hook internal;

public:
	FunctionHook();
	FunctionHook(T function, T hook, const char *name);

	void Rehook();
	void Unhook();

	T Call();
};

template<class T> FunctionHook<T>::FunctionHook() {
	internal = {};
}

template<class T> FunctionHook<T>::FunctionHook(T function, T hook, const char* name) {
	hook_init(&internal, reinterpret_cast<void*>(function), reinterpret_cast<void*>(hook), name);
	Rehook();
}

template<class T> void FunctionHook<T>::Rehook() {
	rehook(&internal);
}

template<class T> void FunctionHook<T>::Unhook() {
	unhook(&internal);
}

template<class T> T FunctionHook<T>::Call() {
	return reinterpret_cast<T>(internal.call_addr);
}