# Minecraft APK Symbol Investigation Guide

This directory contains tools for investigating symbol changes between Minecraft Bedrock versions to identify why newer versions fail to launch.

## Purpose

When Minecraft updates break compatibility with mcpelauncher, it's often due to:
1. **JNI method changes** - Native methods renamed, added, or removed
2. **vtable changes** - C++ virtual tables renamed or restructured
3. **New libraries** - Additional libraries like libpairipcore.so added
4. **Symbol removals** - Previously exported symbols made private

This guide helps identify these changes to create patches for the launcher.

---

## Prerequisites

### Tools Required
```bash
# Ubuntu/Debian
sudo apt install unzip binutils

# Fedora
sudo dnf install unzip binutils

# Arch
sudo pacman -S unzip binutils
```

### APK Files Needed
You need the Minecraft APK files for both:
1. A **working version** (e.g., 1.21.80)
2. A **broken version** (e.g., 1.21.120)

APKs can be extracted from your Android device if you own Minecraft:
```bash
# On Android with ADB
adb shell pm path com.mojang.minecraftpe
adb pull /data/app/.../base.apk minecraft-version.apk
```

---

## Quick Start

### Step 1: Extract Symbols from Both Versions

```bash
chmod +x extract-symbols.sh compare-versions.sh

# Extract symbols from working version
./extract-symbols.sh minecraft-1.21.80.apk x86_64

# Extract symbols from broken version
./extract-symbols.sh minecraft-1.21.120.apk x86_64
```

### Step 2: Compare the Versions

```bash
./compare-versions.sh 1.21.80 1.21.120
```

### Step 3: Review the Analysis

```bash
# Check the comparison report
cat comparison/COMPARISON_REPORT.md

# Review JNI changes
cat comparison/analysis_jni.md

# Review vtable changes
cat comparison/analysis_vtables.md

# Review pairipcore analysis
cat comparison/analysis_pairipcore.md
```

---

## Understanding the Output

### Symbol Files Generated

| File | Contents |
|------|----------|
| `*_dynamic_symbols.txt` | All exported symbols (nm -D) |
| `*_MainActivity.txt` | MainActivity-related symbols |
| `*_AppPlatform.txt` | AppPlatform-related symbols |
| `*_JNI_methods.txt` | Java_* native method symbols |
| `*_vtables.txt` | _ZTV* virtual table symbols |
| `*_system_property.txt` | System property related symbols |

### Comparison Files Generated

| File | Purpose |
|------|---------|
| `diff_MainActivity.txt` | Shows added/removed MainActivity symbols |
| `diff_JNI_methods.txt` | Shows added/removed JNI methods |
| `diff_vtables.txt` | Shows added/removed vtables |
| `analysis_jni.md` | Detailed JNI change analysis |
| `analysis_vtables.md` | Detailed vtable change analysis |

---

## Common Issues and Fixes

### Issue 1: Missing JNI Symbol

**Error:**
```
[JniSupport] Missing native symbol: Java_com_mojang_minecraftpe_MainActivity_nativeXXX
```

**Cause:** The launcher is looking for a JNI symbol that doesn't exist in the new version.

**Investigation:**
```bash
# Check if the method was renamed
grep "nativeXXX" comparison/diff_JNI_methods.txt

# Check what methods were added
grep "^+" comparison/diff_JNI_methods.txt | grep -i "MainActivity"
```

**Fix:** Update `mcpelauncher-client/src/jni/jni_support.cpp` to:
- Remove registration for deleted methods
- Add registration for new methods
- Update method signatures if changed

### Issue 2: vtable Not Found

**Error:**
```
[CorePatches] Failed to patch, vtable _ZTV21AppPlatform_android23 not found
```

**Cause:** The AppPlatform class version number changed (e.g., android23 → android24).

**Investigation:**
```bash
# Check vtable changes
cat comparison/analysis_vtables.md

# Find the new vtable name
grep "AppPlatform" output/1.21.120_vtables.txt
```

**Fix:** Update `mcpelauncher-core` to search for new vtable names.

**Note:** Since Minecraft 1.16.210+, the launcher primarily uses hook-based patching.
The vtable error may be non-fatal.

### Issue 3: libpairipcore.so Crashes

**Error:**
```
dlopen failed: cannot locate symbol '__system_property_foreach' referenced by libpairipcore.so
```

**Cause:** Google's PairIP protection library requires Android system property functions.

**Fix:** Apply the libc-shim patch in `steamdeck-build/libc-shim-patch/`.

---

## Manual Symbol Investigation

### Dump All MainActivity Methods
```bash
nm -D libminecraftpe.so | grep MainActivity
```

### Find Specific Method
```bash
nm -D libminecraftpe.so | grep -i "nativeRegisterThis"
```

### Check vtable Names
```bash
nm -D libminecraftpe.so | grep "_ZTV.*AppPlatform"
```

### List All JNI Methods
```bash
nm -D libminecraftpe.so | grep "^Java_" | sort
```

### Check for pairipcore
```bash
ls -la lib/x86_64/libpairipcore.so
nm -D lib/x86_64/libpairipcore.so 2>/dev/null | head -50
```

---

## Patching the Launcher

After identifying changes, update these files:

### For JNI Changes
- `mcpelauncher-client/src/jni/jni_support.cpp` - Method registration
- `mcpelauncher-client/src/jni/main_activity.cpp` - MainActivity implementation
- `mcpelauncher-client/src/jni/jni_descriptors.cpp` - Class descriptors

### For vtable Changes
- `mcpelauncher-core/src/patch_utils.cpp` - vtable searching
- `mcpelauncher-core/src/minecraft_utils.cpp` - Hook registration

### For System Property Issues
- `libc-shim/src/system_properties.cpp` - Property implementation
- `libc-shim/src/system_properties.h` - Function declarations

---

## Example Workflow

```bash
# 1. Get APKs
adb pull /data/app/com.mojang.minecraftpe-1/base.apk minecraft-1.21.80.apk
adb pull /data/app/com.mojang.minecraftpe-2/base.apk minecraft-1.21.120.apk

# 2. Extract symbols
./extract-symbols.sh minecraft-1.21.80.apk
./extract-symbols.sh minecraft-1.21.120.apk

# 3. Compare
./compare-versions.sh 1.21.80 1.21.120

# 4. Review changes
cat comparison/COMPARISON_REPORT.md

# 5. Check specific issues
# If JNI error:
cat comparison/analysis_jni.md

# If vtable error:
cat comparison/analysis_vtables.md

# 6. Create patches based on findings
# ... (update launcher code)

# 7. Rebuild and test
```

---

## Contributing Findings

If you identify the specific changes causing incompatibility:

1. Document the exact symbol changes
2. Create patches for the affected files
3. Submit findings to:
   - https://github.com/minecraft-linux/mcpelauncher-manifest/issues
   - Include version numbers and symbol diff output

---

## References

- [mcpelauncher JNI Documentation](https://mcpelauncher.readthedocs.io/en/latest/extra/advanced/jni/index.html)
- [libjnivm Repository](https://github.com/ChristopherHX/libjnivm)
- [minecraft-symbols Repository](https://github.com/minecraft-linux/minecraft-symbols)
- [PairIP Core Research](https://github.com/Solaree/pairipcore)
