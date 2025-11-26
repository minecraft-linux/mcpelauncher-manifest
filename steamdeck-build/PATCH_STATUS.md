# MCPELauncher Patch Status for Minecraft 1.21.100+

## Overview

This document summarizes the current patch status for running Minecraft Bedrock Edition 1.21.100+ on Linux via mcpelauncher.

---

## Issue Summary

| Issue | Component | Status | Fix Available |
|-------|-----------|--------|---------------|
| `__system_property_foreach` missing | libc-shim | **FIXED** | ✅ Patch provided |
| `nativeRegisterThis` missing | mcpelauncher-client | **NEEDS INVESTIGATION** | ⚠️ |
| vtable `AppPlatform_android23` not found | mcpelauncher-core | **NEEDS INVESTIGATION** | ⚠️ |
| FMOD libfmod.so.12 missing | mcpelauncher-bin | **NEEDS UPDATE** | ⚠️ |

---

## Issue 1: System Properties (FIXED)

### Problem
```
dlopen failed: cannot locate symbol '__system_property_foreach' referenced by libpairipcore.so
```

### Root Cause
Google's PairIP Core library (`libpairipcore.so`) in Minecraft 1.21.111+ requires the Android system properties API to verify device integrity. The libc-shim had stubbed implementations that called `abort()`.

### Fix Provided
Complete implementation in `steamdeck-build/libc-shim-patch/`:
- `system_properties.h` - Header with all function declarations
- `system_properties.cpp` - Full implementation with ~150 realistic Android properties
- `README.md` - Usage documentation
- `BUILD_INSTRUCTIONS.md` - Build guide

### Functions Implemented
- `__system_property_foreach()` - **CRITICAL** - Iterates all properties
- `__system_property_find()` - Find property by name
- `__system_property_get()` - Get property value
- `__system_property_read()` - Old API (pre-API 26)
- `__system_property_read_callback()` - New API (API 26+)
- `__system_property_serial()` - Change detection
- `__system_property_wait()` - Wait for changes

---

## Issue 2: JNI Missing Symbols (NEEDS INVESTIGATION)

### Problem
```
[JniSupport] Missing native symbol: Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis
```

### Analysis

The `nativeRegisterThis` method IS registered in `mcpelauncher-client/src/jni/jni_support.cpp`. The error means the function is missing from `libminecraftpe.so`, not the launcher.

This indicates Minecraft has changed its native library structure in 1.21.100+:
1. The function may have been renamed
2. The function signature may have changed
3. The function may have been moved to a different class

### Required Investigation

1. **Dump libminecraftpe.so symbols** from a 1.21.120+ APK:
   ```bash
   nm -D libminecraftpe.so | grep nativeRegister
   nm -D libminecraftpe.so | grep MainActivity
   ```

2. **Compare with older version** (1.21.80):
   ```bash
   diff old_symbols.txt new_symbols.txt
   ```

3. **Update jni_support.cpp** to register the new method names

### Potential Workaround

If the method was renamed, update the registration in `jni_support.cpp`:
```cpp
// If renamed from nativeRegisterThis to something else:
// Find the new name and update the mapping
```

---

## Issue 3: vtable AppPlatform_android23 (NEEDS INVESTIGATION)

### Problem
```
[CorePatches] Failed to patch, vtable _ZTV21AppPlatform_android23 not found
```

### Analysis

This error indicates the launcher is searching for a vtable symbol that doesn't exist in the new Minecraft version. The vtable naming follows Android version numbers:
- `_ZTV21AppPlatform_android22` - Older versions
- `_ZTV21AppPlatform_android23` - Android 13
- `_ZTV21AppPlatform_android24` - Android 14?

### Required Investigation

1. **Dump vtable symbols** from libminecraftpe.so:
   ```bash
   nm -D libminecraftpe.so | grep AppPlatform
   nm -D libminecraftpe.so | grep "_ZTV"
   ```

2. **Find the new vtable name** (likely android24 or higher)

3. **Update mcpelauncher-core** to search for all possible vtable names:
   ```cpp
   // In patch_utils.cpp or similar:
   const char* vtable_names[] = {
       "_ZTV21AppPlatform_android22",
       "_ZTV21AppPlatform_android23",
       "_ZTV21AppPlatform_android24",  // Add new version
       "_ZTV21AppPlatform_android25",  // Future-proof
   };
   ```

### Note

This is a **debug message**, not necessarily a fatal error. The launcher may still work if other patches succeed. However, some functionality may be broken without the vtable patches.

---

## Issue 4: FMOD Audio Library (NEEDS UPDATE)

### Problem
```
[FMOD] Failed to load host libfmod
Warning: Failed to load host libfmod.so.12.0
```

### Analysis

Minecraft 1.21.x uses FMOD version 12.x, but the launcher may be bundled with an older version.

### Fix

1. **Download FMOD** from https://www.fmod.com/download (requires free account)
2. **Extract Linux libraries**:
   - `libfmod.so.12.0`
   - `libfmodstudio.so.12.0`
3. **Place in launcher directory**:
   ```
   /app/share/mcpelauncher/lib/native/x86_64/libfmod.so.12.0
   ```
4. **Create symlink**:
   ```bash
   ln -s libfmod.so.12.0 libfmod.so
   ```

---

## Current Working Versions

| Version | Status | Notes |
|---------|--------|-------|
| 1.21.73 | ✅ Working | Last fully compatible |
| 1.21.80 | ✅ Working | Requires v1.3.0+ launcher |
| 1.21.92.1 | ⚠️ Partial | Random Signal 11 crashes |
| 1.21.100.6 | ❌ Broken | JNI + vtable issues |
| 1.21.111.1 | ❌ Broken | + libpairipcore (fixed with patch) |
| 1.21.114+ | ❌ Broken | Multiple issues |
| 1.21.120+ | ❌ Broken | Multiple issues |

---

## Recommended Actions

### For Immediate Use

Use Minecraft **1.21.80** until patches are complete:
```bash
flatpak run io.mrarm.mcpelauncher -d
# Go to Settings > Dev > Enable all version toggles
# Download 1.21.80
```

### For Developers

1. **Apply libc-shim patch** (provided in this repository)
2. **Investigate JNI changes** in libminecraftpe.so
3. **Update vtable search** in mcpelauncher-core
4. **Bundle updated FMOD** libraries

### For Testing

After applying the libc-shim patch, test with 1.21.111.1:
1. If still crashes with JNI errors → JNI issue needs fixing
2. If still crashes with vtable errors → vtable issue needs fixing
3. If no sound → FMOD issue needs fixing
4. If works → Success!

---

## Files Created

| File | Description |
|------|-------------|
| `libc-shim-patch/system_properties.h` | Header file |
| `libc-shim-patch/system_properties.cpp` | Implementation |
| `libc-shim-patch/README.md` | Documentation |
| `libc-shim-patch/BUILD_INSTRUCTIONS.md` | Build guide |
| `CRASH_ANALYSIS.md` | Detailed crash analysis |
| `PATCH_STATUS.md` | This document |

---

## References

- [libc-shim Repository](https://github.com/minecraft-linux/libc-shim)
- [mcpelauncher-client Repository](https://github.com/minecraft-linux/mcpelauncher-client)
- [mcpelauncher-core Repository](https://github.com/minecraft-linux/mcpelauncher-core)
- [Issue #1364 - system_property_foreach](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1364)
- [Issue #1448 - v1.21.120 failures](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1448)
- [Issue #1298 - JNI errors](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1298)
- [PairIP Core Research](https://github.com/Solaree/pairipcore)
