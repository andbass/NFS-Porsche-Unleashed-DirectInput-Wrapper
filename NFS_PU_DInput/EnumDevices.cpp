
#include "EnumDevices.h"
#include "Log.h"
#include "Util.h"

#include <vector>

enum class Version : unsigned {
	A, // Ascii
	W, // Wide - UTF16 (I think?)
};

template <Version>
struct Traits {};

template <>
struct Traits<Version::A> {
	static constexpr Version version = Version::A;

	using DirectInput = LPDIRECTINPUTA;
	using DeviceInstance = DIDEVICEINSTANCEA;
	using EnumDevices = EnumDevicesAProc;
	using Callback = LPDIENUMDEVICESCALLBACKA;
};

template <>
struct Traits<Version::W> {
	static constexpr Version version = Version::W;

	using DirectInput = LPDIRECTINPUTW;
	using DeviceInstance = DIDEVICEINSTANCEW;
	using EnumDevices = EnumDevicesWProc;
	using Callback = LPDIENUMDEVICESCALLBACKW;
};

template <typename TTRAITS>
struct CallbackState {
	std::vector<typename TTRAITS::DeviceInstance> devices;
};

template <typename TTRAITS>
static BOOL __stdcall enumDevicesCallback(typename TTRAITS::DeviceInstance* device, LPVOID context) {
	auto* state = static_cast<CallbackState<TTRAITS>*>(context);
	state->devices.push_back(*device);
	return DIENUM_CONTINUE; // We want every device so never bail early
}

template <typename TTRAITS>
static HRESULT STDMETHODCALLTYPE enumDevicesImpl(typename TTRAITS::EnumDevices origFunc, typename TTRAITS::DirectInput This, DWORD devType, typename TTRAITS::Callback externalCallback, LPVOID context, DWORD flags) {
	// TODO these are kind of hacky
	devType = 4; // DIDEVTYPE_JOYSTICK
	flags |= DIEDFL_ATTACHEDONLY;
	LOG_PRINT("Overriding dev type to %u and adding ATTACHED_ONLY to flags (%#x)", devType, flags);

	CallbackState<TTRAITS> state;
	auto callback = reinterpret_cast<typename TTRAITS::Callback>(&enumDevicesCallback<TTRAITS>); // Shouldn't have to cast here, but it seems to be an issue with the older compiler used in the XP toolchain

	// Call the enumDevices function ourselves just to get a list of devices
	//
	// My original plan was to reorder them to put the wheel at the front,
	// but overiding devType to 4 (DIDEVTYPE_JOYSTICK) was enough
	// to isolate only my wheel and get the game working with it
	//
	// Might be useful in the future
	HRESULT result = origFunc(This, devType, callback, &state, flags);
	if (result != DI_OK) {
		LOG_PRINT("Failed to call EnumDevices version %u", TTRAITS::version);
		return result; // TODO technically don't need this, but if we fail calling it ourselves then :shrug:
	}

	for (unsigned i = 0; i < state.devices.size(); ++i) {
		typename TTRAITS::DeviceInstance& device = state.devices[i];
		if constexpr (TTRAITS::version == Version::A) {
			LOG_PRINT("Version Ascii, Device %d: %s, %s", i, device.tszInstanceName, device.tszProductName);
		} else {
			LOG_PRINT("Version Unicode, Device %d: %S, %S", i, device.tszInstanceName, device.tszProductName);
		}
	}

	// Could replay `state.devices` here, but with the `flags` and `devType` override
	// there is no need (works on my machine TM, you might want to mess with it possibly)
	return origFunc(This, devType, externalCallback, context, flags);
}

HRESULT STDMETHODCALLTYPE customEnumDevicesA(LPDIRECTINPUTA This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) {
	LOG_PRINT("Enum devices was called, dev type = %u, flags = %#x, thread = %u", dwDevType, dwFlags, GetCurrentThreadId());
	return enumDevicesImpl<Traits<Version::A>>(g_origEnumDevicesA, This, dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT STDMETHODCALLTYPE customEnumDevicesW(LPDIRECTINPUTW This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) {
	LOG_PRINT("Enum devices was called, dev type = %u, flags = %#x, thread = %u", dwDevType, dwFlags, GetCurrentThreadId());
	return enumDevicesImpl<Traits<Version::W>>(g_origEnumDevicesW, This, dwDevType, lpCallback, pvRef, dwFlags);
}

EnumDevicesAProc g_origEnumDevicesA = nullptr;
EnumDevicesWProc g_origEnumDevicesW = nullptr;
