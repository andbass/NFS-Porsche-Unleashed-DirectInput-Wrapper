
#include "Log.h"
#include "Hook.h"

#include <string>

#include <Windows.h>
#include <process.h>

void _cdecl exitInstance() {}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reasonForCall, LPVOID lpReserved) {
	switch (reasonForCall) {
	case DLL_PROCESS_ATTACH: {
		// TODO don't use relative path
		g_log.initialize("dinput_wrapper.log");
		LOG_PRINT("Initialized logging");

		DisableThreadLibraryCalls(hModule);
		atexit(exitInstance);
		break;
	}
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}