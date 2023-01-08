
#include "Log.h"
#include "Hook.h"
#include "../minhook/include/MinHook.h"

#include <Windows.h>
#include <process.h>
#include <string>

void _cdecl exitInstance() {
	// TODO probably don't need this, is there a weird default module exit handler though?
}

// N.B. Actual hook entry points are the definitions for `DirectInputCreate*()`
//      in `Hook.cpp`, which the game calls in place of the standard DInput functions
//		This just handles basic setup, like logging
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reasonForCall, LPVOID lpReserved) {
	switch (reasonForCall) {
	case DLL_PROCESS_ATTACH: {
		// TODO don't use relative path
		if (g_log.initialize("dinput_wrapper.log")) {
			LOG_PRINT("Initialized logging");
		} // else, message box error is presented by logger

		if (MH_Initialize() == MH_OK) {
			LOG_PRINT("Initialized MinHook");
		} else {
			LOG_PRINT("Failed to initialize MinHook!");
		}

		DisableThreadLibraryCalls(hModule);
		atexit(exitInstance);
		break;
	}
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}