# libc-shim System Properties Patch for Minecraft 1.21.100+

## Overview

This patch implements the missing Android system properties API functions required by Minecraft Bedrock Edition 1.21.100+ which uses Google's PairIP Core protection library (`libpairipcore.so`).

**This is for legitimate use only** - running a properly purchased copy of Minecraft on Linux via mcpelauncher.

## Problem

Starting with Minecraft 1.21.111, Google's PairIP Core library was integrated. This library requires the Android system properties API to verify device integrity. The current libc-shim implementation stubs these functions, causing crashes:

```
dlopen failed: cannot locate symbol '__system_property_foreach' referenced by libpairipcore.so
```

## Solution

This patch provides a complete implementation of the Android system properties API:

| Function | Status | Description |
|----------|--------|-------------|
| `__system_property_find` | ✅ Implemented | Find property by name |
| `__system_property_get` | ✅ Implemented | Get property value |
| `__system_property_read` | ✅ **NEW** | Old API for reading properties |
| `__system_property_read_callback` | ✅ Enhanced | New API with callback |
| `__system_property_foreach` | ✅ **NEW** | Iterate all properties (critical!) |
| `__system_property_serial` | ✅ **NEW** | Get property serial |
| `__system_property_area_serial` | ✅ **NEW** | Get global serial |
| `__system_property_wait` | ✅ **NEW** | Wait for changes (stub) |

## Files

- `system_properties.h` - Header with function declarations
- `system_properties.cpp` - Complete implementation with ~150 realistic Android properties

## Installation

### Option 1: Replace files in libc-shim

```bash
# Clone libc-shim
git clone https://github.com/minecraft-linux/libc-shim.git
cd libc-shim

# Backup original files
cp src/system_properties.h src/system_properties.h.bak
cp src/system_properties.cpp src/system_properties.cpp.bak

# Copy new implementation
cp /path/to/patch/system_properties.h src/
cp /path/to/patch/system_properties.cpp src/

# Rebuild
mkdir build && cd build
cmake ..
make
```

### Option 2: Apply as Git patch

```bash
cd libc-shim
git apply /path/to/libc-shim-system-properties.patch
```

## Properties Included

The implementation provides ~150 system properties that represent a legitimate Android device (Pixel 6 style), including:

### Build Information
- `ro.build.fingerprint` - Full build fingerprint
- `ro.build.version.sdk` - Android SDK version (34)
- `ro.build.version.release` - Android version (14)
- `ro.build.type` - Build type (user)
- `ro.build.tags` - Build tags (release-keys)

### Device Information
- `ro.product.model` - Device model (Pixel 6)
- `ro.product.brand` - Brand (google)
- `ro.product.device` - Device codename (oriole)
- `ro.product.manufacturer` - Manufacturer (Google)

### Security Properties
- `ro.boot.verifiedbootstate` - Boot state (green)
- `ro.boot.flash.locked` - Bootloader locked (1)
- `ro.secure` - Secure mode (1)
- `ro.debuggable` - Debug mode disabled (0)

### CPU/Architecture
- `ro.product.cpu.abi` - Primary ABI (arm64-v8a)
- `ro.product.cpu.abilist` - All supported ABIs

### And many more...

## How PairIP Core Uses These

PairIP Core calls `__system_property_foreach` to iterate through all properties and verify:

1. Device appears to be a legitimate Android device (not emulator)
2. Bootloader is locked and verified boot is "green"
3. Device is not in debug mode
4. Build fingerprint matches expected format
5. Various other integrity checks

Our implementation provides realistic property values that satisfy these checks while running on a legitimate Linux system.

## Testing

After applying the patch and rebuilding:

```bash
# Test with Minecraft 1.21.111+
flatpak run io.mrarm.mcpelauncher -d

# Check for system property errors in logs
# Should no longer see:
# "dlopen failed: cannot locate symbol '__system_property_foreach'"
```

## Troubleshooting

### Still crashing after patch?

1. **Check launcher version**: Ensure you're using the latest nightly build
2. **Check other missing symbols**: There may be additional JNI or vtable issues
3. **Check FMOD**: Ensure libfmod.so.12 is present

### Properties not being read?

Enable debug logging to see which properties are being accessed:

```cpp
// In system_properties.cpp, add at the start of __system_property_foreach:
fprintf(stderr, "[DEBUG] __system_property_foreach called\n");
```

## License

MIT License - Same as libc-shim

## References

- [Android Bionic system_property_api.cpp](https://android.googlesource.com/platform/bionic/+/master/libc/bionic/system_property_api.cpp)
- [PairIP Core Research](https://github.com/Solaree/pairipcore)
- [libc-shim Repository](https://github.com/minecraft-linux/libc-shim)
- [mcpelauncher Issue #1364](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1364)
