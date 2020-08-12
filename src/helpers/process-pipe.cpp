/*
Windows Audio Session Capture Plugin for OBS
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
#include "process-pipe.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <util/bmem.h>
#include <util/platform.h>
#include <util/windows/WinHandle.hpp>

#include <util/base.h>

#include "../plugin-macros.generated.h"

ProcessPipe::ProcessPipe(char *command)
{
	WinHandle hStdoutWr;

	// Little bit nasty
	SECURITY_ATTRIBUTES attributes = {};
	attributes.nLength = sizeof SECURITY_ATTRIBUTES;
	attributes.bInheritHandle = true;
	attributes.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&hStdout, &hStdoutWr, &attributes, 0)) {
		throw GetLastError();
	}

	if (!SetHandleInformation(hStdout, HANDLE_FLAG_INHERIT, 0)) {
		throw GetLastError();
	}

	PROCESS_INFORMATION procInfo = {};
	STARTUPINFOW startInfo = {};

	startInfo.cb = sizeof STARTUPINFOW;
	startInfo.hStdError = nullptr;
	startInfo.hStdInput = nullptr;
	startInfo.hStdOutput = hStdoutWr;
	startInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_FORCEOFFFEEDBACK;

	LPWSTR wCommand = nullptr;
	os_utf8_to_wcs_ptr(command, 0, &wCommand);

	if (!CreateProcessW(nullptr, wCommand, nullptr, nullptr, true,
			    CREATE_NO_WINDOW, nullptr, nullptr, &startInfo,
			    &procInfo)) {
		bfree(wCommand);
		throw GetLastError();
	}

	process = procInfo.hProcess;
	CloseHandle(procInfo.hThread);

	bfree(wCommand);
}

// Who needs istreams anyway?
size_t ProcessPipe::Read(char *buffer, size_t length)
{
	DWORD bytesRead;
	if(ReadFile(hStdout, buffer, static_cast<DWORD>(length), &bytesRead,
		     nullptr)) {
		return bytesRead;
	}

	return 0;
}

void ProcessPipe::CreateChildProcess(char *command)
{

}