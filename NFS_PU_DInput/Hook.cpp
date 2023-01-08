
#include "Hook.h"
#include "EnumDevices.h"
#include "Log.h"

#include "../minhook/include/MinHook.h"

#include <Guiddef.h>
#include <unordered_set>

enum class InitState {
	NEED_TO_INIT,
	DONE,
	FAILED,
};

static InitState g_initState = InitState::NEED_TO_INIT;
static DirectInputCreateProc g_directInputCreateEx = nullptr;
static std::unordered_set<void*> g_hookedFuncs;

void prepareForHooks() {
	if (g_initState != InitState::NEED_TO_INIT) {
		// If we already initialized (either success or failure), don't do it again
		return;
	}

	LOG_PRINT("Beginning initialization");

	// TODO don't load dinput this way - super brittle
	HMODULE dinputDll = LoadLibraryA("C:/Windows/System32/dinput.dll");
	if (dinputDll == nullptr) {
		LOG_PRINT("Could not open dinput.dll from 'C:/Windows/System32/'");
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
	// TODO thread safety?
	// We have to check that we don't install hooks more than once -
	// Porsche Unleashed for some reason calls `DirectInputCreate*()` multiple times
	if (IsEqualIID(dinputId, IID_IDirectInputA)) {
		LOG_PRINT("Installing hooks for ascii version");

		auto* dinputAsc = static_cast<LPDIRECTINPUTA>(*rawDInput);
	
		EnumDevicesAProc enumDevicesA = dinputAsc->lpVtbl->EnumDevices;
		LOG_PRINT("Address of EnumDevices: %p", enumDevicesA);

		if (g_hookedFuncs.count(enumDevicesA) == 0) {
			// TODO check MH return values here
			MH_CreateHook(enumDevicesA, customEnumDevicesA, reinterpret_cast<void**>(&g_origEnumDevicesA));
			MH_EnableHook(enumDevicesA);

			g_hookedFuncs.emplace(enumDevicesA);
			LOG_PRINT("Hook installed for EnumDevicesA");
		} else {
			LOG_PRINT("Function already hooked - skipping");
		}

		return true;
	} else if (IsEqualIID(dinputId, IID_IDirectInputW)) {
		LOG_PRINT("Installing hooks for unicode version");

		auto* dinputUni = static_cast<LPDIRECTINPUTW>(*rawDInput);

		EnumDevicesWProc enumDevicesW = dinputUni->lpVtbl->EnumDevices;
		LOG_PRINT("Address of EnumDevices: %p", enumDevicesW);

		if (g_hookedFuncs.count(enumDevicesW) == 0) {
			// TODO check MH return values here
			MH_CreateHook(enumDevicesW, customEnumDevicesW, reinterpret_cast<void**>(&g_origEnumDevicesW));
			MH_EnableHook(enumDevicesW);

			g_hookedFuncs.emplace(enumDevicesW);
			LOG_PRINT("Hook installed for EnumDevicesW");
		} else {
			LOG_PRINT("Function already hooked - skipping");
		}

		return true;
	} else {
		LOG_PRINT("Unrecognized IID found when installing hooks, bailing");
		return false;
	}
}

//
// Functions below are called by the game in place of the standard DInput setup functions
//

HRESULT WINAPI DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID* lplpDD, LPUNKNOWN punkOuter) {
	LOG_PRINT("Invoked with version %#x", dwVersion);

	prepareForHooks();
	if (g_initState == InitState::FAILED) {
		LOG_PRINT("Initialization failed, triggering process exit");
		exit(1); // TODO is this safe?
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
