#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "audio-hook.hpp"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		try {
			Capture::Start();
		} catch (const char *err) {
			// LOG HERE
			return false;
		}

		HMODULE hmod;
		/* Prevents the library from being unloaded by FreeLibrary. This is the
		 * recommended method post-XP, where calling LoadLibrary inside DllMain
		 * is highly discouraged (though probably OK in graphics-hook) */
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
						GET_MODULE_HANDLE_EX_FLAG_PIN,
					reinterpret_cast<LPCWSTR>(hinst),
					&hmod)) {
			// LOG HERE
			return false;
		}
	}

	// Windows cleans us up on detach, no need to do it ourselves

	return true;
}