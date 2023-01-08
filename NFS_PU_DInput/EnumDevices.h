#pragma once

#define INITGUID
#define CINTERFACE

#include <Windows.h>
#include <dinput.h>

using EnumDevicesAProc = HRESULT(STDMETHODCALLTYPE*)(LPDIRECTINPUTA This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags);
using EnumDevicesWProc = HRESULT(STDMETHODCALLTYPE*)(LPDIRECTINPUTW This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags);

extern EnumDevicesAProc g_origEnumDevicesA;
extern EnumDevicesWProc g_origEnumDevicesW;

HRESULT STDMETHODCALLTYPE customEnumDevicesA(LPDIRECTINPUTA This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags);
HRESULT STDMETHODCALLTYPE customEnumDevicesW(LPDIRECTINPUTW This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags);

