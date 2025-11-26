# MCPELauncher APK Version Analysis and Steam Deck Build Guide

## Executive Summary

This document provides a comprehensive analysis of the mcpelauncher codebase regarding APK version checking, download mechanisms, and version restrictions. It also includes instructions for building an unrestricted offline Flatpak for Steam Deck.

---

## 1. APK Version Checking Architecture

### Components Involved

| Component | Location | Purpose |
|-----------|----------|---------|
| `mcpelauncher-ui-qt` | External repo | Qt-based UI for version management |
| `versionmanager.cpp/h` | mcpelauncher-ui-qt | Stores/retrieves version metadata |
| `archivalversionlist.cpp` | mcpelauncher-ui-qt | Manages historical version lists |
| `googleversionchannel.cpp` | mcpelauncher-ui-qt | Google Play Store integration |
| `mcpelauncher-apkinfo` | Submodule | APK metadata extraction |
| `axml-parser` | Submodule | Android XML parsing |
| `mcpelauncher-versiondb` | External repo | Version compatibility database |
| `google-play-api` | External repo | Google Play API integration |

### Version Data Flow

```
Google Play Store API
        ↓
googleversionchannel.cpp (fetch latest version)
        ↓
versionmanager.cpp (store version metadata)
        ↓
mcpelauncher-versiondb (compatibility check)
        ↓
UI (show/hide based on settings)
```

---

## 2. Version Restriction Mechanisms

### 2.1 Compile-Time Flag

**Location**: `mcpelauncher-ui-qt/main.cpp`

```cpp
#ifdef LAUNCHER_DISABLE_DEV_MODE
bool LauncherSettings::disableDevMode = 1;  // Dev tab hidden
#else
bool LauncherSettings::disableDevMode = 0;  // Dev tab visible
#endif
```

**Impact**: When `LAUNCHER_DISABLE_DEV_MODE` is defined, the Dev tab in Settings is hidden, preventing access to:
- Show Unverified Versions
- Show Incompatible Versions
- Show Beta Versions

### 2.2 Runtime Settings

**Location**: `mcpelauncher-ui-qt/launchersettings.h`

| Setting | Default | Description |
|---------|---------|-------------|
| `showBetaVersions` | `false` | Show beta Minecraft versions |
| `showUnverified` | `false` | Show versions not verified to work |
| `showUnsupported` | `false` | Show architecture-incompatible versions |

### 2.3 UI Visibility Control

**Location**: `mcpelauncher-ui-qt/qml/LauncherSettingsWindow.qml`

```qml
// Dev tab visibility controlled by:
visible: !(DISABLE_DEV_MODE)
```

### 2.4 Version Database Filtering

**Location**: `mcpelauncher-versiondb` repository

The version database categorizes versions as:
- **Supported**: Tested and working
- **Allowed**: May work, not fully tested
- **Unknown**: Not tested, may have issues

---

## 3. How to Remove Restrictions

### Option 1: Runtime Flag (Temporary)

Run the launcher with `-d` flag:
```bash
# Flatpak
flatpak run io.mrarm.mcpelauncher -d

# AppImage
./Minecraft_Bedrock_Launcher.AppImage -d

# Native
mcpelauncher-ui-qt -d
```

### Option 2: Compile Without Restrictions (Permanent)

Remove or unset `LAUNCHER_DISABLE_DEV_MODE` in CMakeLists.txt:
```cmake
# Remove this line or set to OFF:
# add_definitions(-DLAUNCHER_DISABLE_DEV_MODE)
```

### Option 3: Modify Settings Defaults (Permanent)

Edit `launchersettings.h`:
```cpp
// Change defaults to true:
bool showBetaVersions = true;
bool showUnverified = true;
bool showUnsupported = true;
```

---

## 4. Latest Minecraft Version Compatibility

### Supported Versions (as of November 2025)

| Platform | Architecture | Max Version | Notes |
|----------|--------------|-------------|-------|
| Linux | x86_64 | 1.21.73 | OpenGL ES 3.0+ required |
| Linux | x86 | 1.21.73 | OpenGL ES 3.0+ required |
| Linux | arm64 | 1.21.73 | OpenGL ES 3.0+ required |
| macOS | x86_64 | 1.21.73 | |
| macOS | arm64 | 1.21.73 | Experimental |

### Known Issues with Latest Versions

| Version | Issue | Workaround |
|---------|-------|------------|
| 1.21.60.10 | Crash if `vortex.data.microsoft.com` blocked | Allowlist domain in DNS |
| 1.21.92.1 | Signal 11 crash on SteamOS | Use nightly build, wait for patch |
| 1.20.31+ | Requires OpenGL ES 3.0 | Use `LIBGL_ALWAYS_SOFTWARE=1` |
| All | Cannot receive Xbox Live invites | No workaround |
| All | May fail to join via Xbox friends | No workaround |

### Patches Required for Latest Version

1. **DNS Configuration**: Ensure `vortex.data.microsoft.com` is not blocked
2. **Graphics Driver**: OpenGL ES 3.0+ compatible driver
3. **Launcher Version**: v1.5.1+ or nightly build
4. **Steam Deck Specific**: Use mesa with OpenGL ES 3.0 support

---

## 5. Steam Deck Flatpak Build Instructions

### Prerequisites

```bash
# Install Flatpak builder tools
sudo apt install flatpak flatpak-builder

# Add Flathub repository
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-add --if-not-exists flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo

# Install runtime dependencies
flatpak install flathub org.kde.Platform//6.7
flatpak install flathub org.kde.Sdk//6.7
```

### Build Command

```bash
# Clone the Flatpak manifest
git clone https://github.com/flathub/io.mrarm.mcpelauncher.git
cd io.mrarm.mcpelauncher

# Build with dev mode enabled (modify manifest first)
flatpak-builder --force-clean build-dir io.mrarm.mcpelauncher.json

# Create offline bundle
flatpak build-bundle ~/.local/share/flatpak/repo \
    mcpelauncher-steamdeck.flatpak \
    io.mrarm.mcpelauncher
```

### Offline Installation on Steam Deck

```bash
# Copy the .flatpak bundle to Steam Deck via USB
# Then in Desktop Mode on Steam Deck:
flatpak install --user mcpelauncher-steamdeck.flatpak
```

---

## 6. Recommended Configuration for Steam Deck

### Environment Variables

Create `~/.var/app/io.mrarm.mcpelauncher/config/environment`:
```bash
# Force software rendering if GPU issues occur
# LIBGL_ALWAYS_SOFTWARE=1

# Ensure proper display
SDL_VIDEODRIVER=x11

# Fix audio
PULSE_LATENCY_MSEC=60
```

### Launch Options in Steam

```
flatpak run io.mrarm.mcpelauncher -d %command%
```

### Network Requirements

Ensure these domains are accessible:
- `vortex.data.microsoft.com` (telemetry, required)
- `login.live.com` (Xbox authentication)
- `xsts.auth.xboxlive.com` (Xbox authentication)
- `play.googleapis.com` (Google Play download)

---

## 7. Files Modified/Created

| File | Change |
|------|--------|
| `steamdeck-build/ANALYSIS.md` | This document |
| `steamdeck-build/build-flatpak.sh` | Build script |
| `steamdeck-build/io.mrarm.mcpelauncher.steamdeck.json` | Modified manifest |
| `steamdeck-build/patches/enable-all-versions.patch` | Patch to enable all versions |

---

## Sources

- [mcpelauncher-manifest GitHub](https://github.com/minecraft-linux/mcpelauncher-manifest)
- [Version Compatibility Issue #797](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/797)
- [Steam Deck Crash Issue #1407](https://github.com/minecraft-linux/mcpelauncher-manifest/issues/1407)
- [Flathub mcpelauncher](https://flathub.org/en/apps/io.mrarm.mcpelauncher)
- [Official Wiki](https://minecraft-linux.github.io/)
