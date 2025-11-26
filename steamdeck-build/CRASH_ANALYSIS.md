# MCPELauncher Latest Version Crash Analysis and Required Patches

## Executive Summary

This document analyzes all known crashes and issues with the latest Minecraft Bedrock APK versions (1.21.x) on mcpelauncher and documents the required patches and fixes.

---

## 1. Critical Issues by Minecraft Version

### Version 1.21.111.1+ (Copper Age Update)

| Issue | Root Cause | Status | Fix |
|-------|-----------|--------|-----|
| `dlopen failed: cannot locate symbol '__system_property_foreach'` | libpairipcore.so requires Android system property API | **OPEN** | Implement in libc-shim |
| Missing JNI symbol `nativeRegisterThis` | New JNI methods in MainActivity | **OPEN** | Update JNI bindings |
| vtable `_ZTV21AppPlatform_android23` not found | Changed Android platform version | **OPEN** | Update core patches |

### Version 1.21.92.1 - 1.21.100.x

| Issue | Root Cause | Status | Fix |
|-------|-----------|--------|-----|
| Signal 11 (SIGSEGV) after 90% loading | Memory corruption in mutex operations | **OPEN** | Awaiting investigation |
| Crashes on Intel HD Graphics 4400 | Driver compatibility | Partial | Use newer mesa drivers |

### Version 1.21.60.10+

| Issue | Root Cause | Status | Fix |
|-------|-----------|--------|-----|
| Crash when DNS blocks telemetry | Network access to `vortex.data.microsoft.com` | **FIXED** | Allowlist domain |
| FMOD audio failures | Missing libfmod.so.12 | Partial | Bundle correct FMOD version |

### Version 1.21.40 - 1.21.60

| Issue | Root Cause | Status | Fix |
|-------|-----------|--------|-----|
| Multiplayer crashes when others join | Network handling bug | **FIXED in v1.5.1** | Update launcher |

---

## 2. Technical Analysis: libpairipcore.so

### What is libpairipcore.so?

`libpairipcore.so` is **Google's anti-piracy protection library** (PairIP Core) added to Minecraft starting around version 1.21.111. It performs:

- APK signature verification
- Device integrity checks
- Rooted device detection
- Anti-debugging measures
- System property validation

### Required Android Functions

The library dynamically imports these Android system functions:

```c
// Currently MISSING in libc-shim (cause crashes):
__system_property_foreach()   // Iterate all properties
__system_property_read()      // Read property value

// Added in Oct 2024 (working):
__system_property_read_callback()  // Read with callback
```

### Current libc-shim Implementation

**Location**: `minecraft-linux/libc-shim/src/common.cpp`

```cpp
// PROBLEM: These are stubbed to abort()
{"__system_property_foreach", (void*)stub},  // Calls abort()!
{"__system_property_read", (void*)stub},     // Calls abort()!
```

### Required Patch: Implement __system_property_foreach

```cpp
// Proposed implementation for libc-shim
#include <map>
#include <string>
#include <mutex>

// Simple property storage (read-only for Minecraft's purposes)
static std::map<std::string, std::string> fake_properties = {
    {"ro.build.version.sdk", "30"},
    {"ro.product.cpu.abi", "x86_64"},
    {"ro.build.type", "user"},
    {"ro.build.fingerprint", "google/generic_x86_64/generic_x86_64:11/RSR1.201013.001/6903271:user/release-keys"},
    {"ro.hardware", "generic_x86_64"},
    {"ro.product.model", "Linux Desktop"},
    {"ro.product.brand", "generic"},
    {"ro.product.device", "generic_x86_64"},
    {"ro.product.name", "sdk_gphone_x86_64"},
    {"gsm.version.baseband", "1.0"},
    {"persist.sys.language", "en"},
    {"persist.sys.country", "US"},
};

struct prop_info {
    const char* name;
    const char* value;
};

int __system_property_foreach(
    void (*propfn)(const prop_info* pi, void* cookie),
    void* cookie
) {
    for (const auto& [key, value] : fake_properties) {
        prop_info pi = {key.c_str(), value.c_str()};
        propfn(&pi, cookie);
    }
    return 0;
}

int __system_property_read(
    const prop_info* pi,
    char* name,
    char* value
) {
    if (pi && pi->name && pi->value) {
        if (name) strncpy(name, pi->name, PROP_NAME_MAX);
        if (value) strncpy(value, pi->value, PROP_VALUE_MAX);
        return strlen(pi->value);
    }
    return 0;
}
```

---

## 3. Technical Analysis: JNI Symbol Issues

### Missing MainActivity Symbols (1.21.100+)

The error indicates new JNI native methods:
```
[JniSupport] Missing native symbol: Java_com_mojang_minecraftpe_MainActivity_nativeRegisterThis
```

### Root Cause

Minecraft has added new Java native methods that require C++ implementations. The launcher's `jni_support.cpp` needs to be updated to register these new methods.

### Required Files to Update

| File | Repository | Change Needed |
|------|------------|---------------|
| `jni_support.cpp` | mcpelauncher-client | Add new JNI method registrations |
| `main_activity.cpp` | mcpelauncher-client | Implement nativeRegisterThis |

---

## 4. Technical Analysis: vtable Patch Failures

### Error Message
```
[CorePatches] Failed to patch, vtable _ZTV21AppPlatform_android23 not found
```

### Root Cause

Minecraft has updated its internal Android platform version number from 22 to 23. The launcher's binary patches look for specific symbol names that have changed.

### Required Patch

**Location**: `mcpelauncher-core/src/core_patches.cpp`

Update the vtable search to include the new version:
```cpp
// Look for both old and new vtable names
const char* vtable_names[] = {
    "_ZTV21AppPlatform_android22",  // Old
    "_ZTV21AppPlatform_android23",  // New (1.21.100+)
    "_ZTV21AppPlatform_android24",  // Future-proofing
};
```

---

## 5. Technical Analysis: FMOD Audio Issues

### Error Message
```
[FMOD] Failed to load host libfmod
Warning: Failed to load host libfmod.so.12.0
```

### Root Cause

Minecraft 1.21.x uses FMOD version 12.0, but the launcher may be bundled with an older version.

### Required Files

| File | Version | Location |
|------|---------|----------|
| `libfmod.so.12.0` | FMOD 2.02.x | `/app/share/mcpelauncher/lib/native/x86_64/` |
| `libfmodstudio.so.12.0` | FMOD 2.02.x | `/app/share/mcpelauncher/lib/native/x86_64/` |

### Solution

Download FMOD from https://www.fmod.com/download and extract the Linux libraries.

---

## 6. Version-Specific Crash Matrix

| Version | libc-shim | JNI | Patches | FMOD | DNS | Status |
|---------|-----------|-----|---------|------|-----|--------|
| 1.21.73 | ✓ | ✓ | ✓ | ✓ | ✓ | **Working** |
| 1.21.80 | ✓ | ✓ | ✓ | ✓ | ✓ | **Working** |
| 1.21.92.1 | ✓ | ~ | ~ | ✓ | ✓ | **Partial** |
| 1.21.100.6 | ✗ | ✗ | ✗ | ✓ | ✓ | **Broken** |
| 1.21.111.1 | ✗ | ✗ | ✗ | ~ | ✓ | **Broken** |
| 1.21.114.1 | ✗ | ✗ | ✗ | ~ | ✓ | **Broken** |
| 1.21.120.4 | ✗ | ✗ | ✗ | ✗ | ✓ | **Broken** |
| 1.21.121.1 | ✗ | ✗ | ✗ | ✗ | ✓ | **Broken** |
| 1.21.123.2 | ✗ | ✗ | ✗ | ✗ | ✓ | **Broken** |

Legend: ✓ = Working, ~ = Partial, ✗ = Broken

---

## 7. Required Patches Summary

### Priority 1: Critical (Blocking all 1.21.100+ versions)

1. **libc-shim: Implement `__system_property_foreach`**
   - Repository: `minecraft-linux/libc-shim`
   - File: `src/common.cpp`
   - Status: Not implemented (stubbed to abort)

2. **libc-shim: Implement `__system_property_read`**
   - Repository: `minecraft-linux/libc-shim`
   - File: `src/common.cpp`
   - Status: Not implemented (stubbed to abort)

### Priority 2: High (Blocking most 1.21.100+ features)

3. **mcpelauncher-client: Add new JNI methods**
   - Repository: `minecraft-linux/mcpelauncher-client`
   - Files: `jni_support.cpp`, `main_activity.cpp`
   - Status: Missing new method registrations

4. **mcpelauncher-core: Update vtable patches**
   - Repository: `minecraft-linux/mcpelauncher-core`
   - File: `src/core_patches.cpp`
   - Status: Looking for old symbol names

### Priority 3: Medium (Audio issues)

5. **mcpelauncher-linux-bin: Update FMOD libraries**
   - Repository: `minecraft-linux/mcpelauncher-linux-bin`
   - Files: `lib/native/*/libfmod*.so`
   - Status: Outdated FMOD version

---

## 8. Temporary Workarounds

### For Version 1.21.100+ Crashes

**Current workaround**: Use Minecraft version 1.21.80 or earlier until patches are available.

```bash
# In launcher with -d flag, download 1.21.80 specifically
flatpak run io.mrarm.mcpelauncher -d
# Go to Settings > Dev > Enable all version toggles
# Download 1.21.80 instead of latest
```

### For DNS-Related Crashes (1.21.60+)

Ensure `vortex.data.microsoft.com` is accessible:
```bash
# Test DNS resolution
nslookup vortex.data.microsoft.com

# If using Pi-hole or similar, allowlist:
# vortex.data.microsoft.com
```

### For Graphics Issues

```bash
# Force software rendering
LIBGL_ALWAYS_SOFTWARE=1 flatpak run io.mrarm.mcpelauncher -d

# Or set Mesa overrides
MESA_GL_VERSION_OVERRIDE=4.6 flatpak run io.mrarm.mcpelauncher -d
```

---

## 9. Recommended Actions

### For Users

1. **Use Minecraft 1.21.80** until patches for newer versions are released
2. **Use the nightly build** from Flathub Beta for latest fixes
3. **Ensure network access** to required Microsoft domains
4. **Report issues** on GitHub with full logs

### For Developers

1. **Priority**: Implement `__system_property_foreach` in libc-shim
2. **Update**: JNI bindings for new MainActivity methods
3. **Fix**: vtable symbol names in core patches
4. **Bundle**: Updated FMOD libraries

---

## 10. Related GitHub Issues

| Issue | Version | Status | Description |
|-------|---------|--------|-------------|
| [#1364](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1364) | 1.21.111.1 | Open | Missing `__system_property_foreach` |
| [#1448](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1448) | 1.21.120.4 | Open | JNI and vtable failures |
| [#1484](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1484) | 1.21.121+ | Open | Crashes on launch |
| [#1298](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1298) | 1.21.100.6 | Open | JNI errors, exit code 9 |
| [#1183](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1183) | 1.21.90.3 | Open | Signal 11 SIGSEGV |
| [#1088](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1088) | 1.21.60.10 | Closed | DNS blocking crash |
| [#797](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/797) | All | Open | Version compatibility tracking |

---

## 11. Sources

- [libc-shim Repository](https://github.com/minecraft-linux/libc-shim)
- [Android Bionic Source (system_property_api.cpp)](https://android.googlesource.com/platform/bionic/+/master/libc/bionic/system_property_api.cpp)
- [PairIP Core Research](https://github.com/Solaree/pairipcore)
- [mcpelauncher JNI Documentation](https://mcpelauncher.readthedocs.io/en/latest/extra/advanced/jni/index.html)
- [FMOD Downloads](https://www.fmod.com/download)
