#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cwchar>

#define EVENT_FLAGS (EVENT_MODIFY_STATE | SYNCHRONIZE)
#define MUTEX_FLAGS (SYNCHRONIZE)

static inline HANDLE CreateEventWithId(const wchar_t* name, DWORD id, bool manualReset = false, bool initialState = false) {
	wchar_t newName[64];
	swprintf(newName, 64, L"%s%lu", name, id);
	return CreateEventW(nullptr, manualReset, initialState, newName);
}

static inline HANDLE OpenEventWithId(const wchar_t* name, DWORD id) {
	wchar_t newName[64];
	swprintf(newName, 64, L"%s%lu", name, id);
	return OpenEventW(EVENT_FLAGS, false, newName);
}

static inline HANDLE CreateMutexWithId(const wchar_t *name, DWORD id)
{
	wchar_t newName[64];
	swprintf(newName, 64, L"%s%lu", name, id);
	return CreateMutexW(nullptr, false, newName);
}

static inline HANDLE OpenMutexWithId(const wchar_t* name, DWORD id) {
	wchar_t newName[64];
	swprintf(newName, 64, L"%s%lu", name, id);
	return OpenMutexW(MUTEX_FLAGS, false, newName);
}

static inline bool Signaled(HANDLE event) {
	return event && event != INVALID_HANDLE_VALUE &&
	       WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
}