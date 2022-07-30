#pragma once

#define INITGUID
#define CINTERFACE

#include <Windows.h>
#include <dinput.h>

using DirectInputCreateProc = HRESULT(WINAPI*)(HINSTANCE hinst, DWORD dwVersion, REFIID riid, LPVOID* lplpDirectInput, LPUNKNOWN punkOuter);

void prepareForHooks();
bool installHooksOnDInput(REFIID dinputId, LPVOID* rawDInput);
