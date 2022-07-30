
#include "Hook.h"
#include "EnumDevices.h"
#include "Log.h"

#include <Guiddef.h>

enum class InitState {
	NEED_TO_INIT,
	DONE,
	FAILED,
};

static InitState g_initState = InitState::NEED_TO_INIT;
static DirectInputCreateProc g_directInputCreateEx = nullptr;

void prepareForHooks() {
	if (g_initState != InitState::NEED_TO_INIT) {
		return;
	}

	LOG_PRINT("Beginning initialization");

	// TODO any other way to not recursively load ourselves?
	HMODULE dinputDll = LoadLibraryA("C:/Windows/System32/dinput.dll");
	if (dinputDll == nullptr) {
		LOG_PRINT("Could not open dinput.dll");
		g_initState = InitState::FAILED;
		return;
	}

	g_directInputCreateEx = reinterpret_cast<DirectInputCreateProc>(GetProcAddress(dinputDll, "DirectInputCreateEx"));
	if (g_directInputCreateEx == nullptr) {
		LOG_PRINT("Could not load create function");
		g_initState = InitState::FAILED;
		return;
	}

	g_initState = InitState::DONE;
}

bool installHooksOnDInput(REFIID dinputId, LPVOID* rawDInput) {
	if (IsEqualIID(dinputId, IID_IDirectInputA)) {
		LOG_PRINT("Installing hooks for ascii version");

		auto* dinputAsc = static_cast<LPDIRECTINPUTA>(*rawDInput);


		return true;
	} else if (IsEqualIID(dinputId, IID_IDirectInputW)) {
		LOG_PRINT("Installing hooks for unicode version");

		auto* dinputUni = static_cast<LPDIRECTINPUTW>(*rawDInput);
		return true;
	} else {
		LOG_PRINT("Unrecognized IID found when installing hooks, exiting");
		return false;
	}
}

HRESULT WINAPI DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID* lplpDD, LPUNKNOWN punkOuter) {
	LOG_PRINT("Invoked with version %#x", dwVersion);

	prepareForHooks();
	if (g_initState == InitState::FAILED) {
		LOG_PRINT("Initialization failed, triggering process exit");
		exit(1);
	}

	HRESULT result = g_directInputCreateEx(hinst, dwVersion, riid, lplpDD, punkOuter);
	if (result == DI_OK) {
		LOG_PRINT("Call to actual dinput interface creation successful, now installing hooks on interface %p", *lplpDD);
		if (!installHooksOnDInput(riid, lplpDD)) {
			LOG_PRINT("Failed to install hooks on the DirectInput interface, triggering process exit");
			exit(1);
		}
	} else {
		LOG_PRINT("DInput creation was not successful, not installing hooks but letting program proceed");
	}
	
	return result;
}

HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTA* lplpDirectInput, LPUNKNOWN punkOuter) {
	LOG_PRINT("Invoked with version %#x", dwVersion);
	return DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputA, (LPVOID*)lplpDirectInput, punkOuter);
}

HRESULT WINAPI DirectInputCreateW(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUTW* lplpDirectInput, LPUNKNOWN punkOuter){
	LOG_PRINT("Invoked with version %#x", dwVersion);
	return DirectInputCreateEx(hinst, dwVersion, IID_IDirectInputW, (LPVOID*)lplpDirectInput, punkOuter);
}
