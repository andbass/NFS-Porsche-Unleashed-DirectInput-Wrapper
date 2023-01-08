# NFS Porsche Unleashed DirectInput Wrapper
This is a very hastily written wrapper around DirectInput that I wrote
over the summer to allow NFS PU to use game controllers and steering wheels
when running under modern Windows.

The core issue is that NFS PU invokes `enumDevices()` with a bad device filter, and thus on modern Windows gets back a strange list of devices. This wrapper forces the device filter param to `DIDEVTYPE_JOYSTICK`. Assuming you only have
one controller or wheel device pluggged in (since the game also has issue with multiple devices), it should work and be configurable in-game.

This makes use of the https://github.com/TsudaKageyu/minhook library (license is included within the `minhook` folder).
