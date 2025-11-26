# Building mcpelauncher with System Properties Patch

## Complete Build Guide for Minecraft 1.21.100+ Support

This guide explains how to build mcpelauncher with the system properties patch to support Minecraft Bedrock Edition 1.21.100+.

---

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake git \
    libssl-dev libcurl4-openssl-dev \
    libpng-dev libuv1-dev libzip-dev \
    libprotobuf-dev protobuf-compiler \
    qt6-base-dev qt6-declarative-dev qt6-webengine-dev \
    libpulse-dev libasound2-dev \
    libx11-dev libxi-dev libxcursor-dev \
    libgl1-mesa-dev libegl1-mesa-dev \
    ninja-build
```

### Fedora
```bash
sudo dnf install -y \
    gcc gcc-c++ cmake git ninja-build \
    openssl-devel libcurl-devel \
    libpng-devel libuv-devel libzip-devel \
    protobuf-devel protobuf-compiler \
    qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtwebengine-devel \
    pulseaudio-libs-devel alsa-lib-devel \
    libX11-devel libXi-devel libXcursor-devel \
    mesa-libGL-devel mesa-libEGL-devel
```

### Steam Deck (SteamOS)
```bash
# Disable read-only filesystem temporarily
sudo steamos-readonly disable

# Install development tools
sudo pacman -S --needed \
    base-devel cmake git ninja \
    openssl curl libpng libuv libzip \
    protobuf qt6-base qt6-declarative qt6-webengine \
    pulseaudio alsa-lib \
    libx11 libxi libxcursor \
    mesa

# Re-enable read-only after building
# sudo steamos-readonly enable
```

---

## Step 1: Clone Repositories

```bash
# Create working directory
mkdir -p ~/mcpelauncher-build
cd ~/mcpelauncher-build

# Clone the main manifest
git clone --recursive https://github.com/minecraft-linux/mcpelauncher-manifest.git
cd mcpelauncher-manifest

# Initialize all submodules
git submodule update --init --recursive
```

---

## Step 2: Apply System Properties Patch

```bash
# Navigate to libc-shim
cd libc-shim

# Backup original files
cp src/system_properties.h src/system_properties.h.original
cp src/system_properties.cpp src/system_properties.cpp.original

# Copy the patched files
cp /path/to/steamdeck-build/libc-shim-patch/system_properties.h src/
cp /path/to/steamdeck-build/libc-shim-patch/system_properties.cpp src/

# Return to manifest root
cd ..
```

---

## Step 3: Build the Launcher

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DGAMEWINDOW_SYSTEM=SDL3 \
    -DENABLE_DEV_PATHS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build (use number of CPU cores)
make -j$(nproc)

# Install (optional)
sudo make install
```

---

## Step 4: Build the UI

```bash
cd ~/mcpelauncher-build

# Clone UI manifest
git clone --recursive https://github.com/minecraft-linux/mcpelauncher-ui-manifest.git
cd mcpelauncher-ui-manifest

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir -p build
cd build

# Configure - IMPORTANT: Disable dev mode restriction
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
make -j$(nproc)

# Install
sudo make install
```

---

## Step 5: Test the Build

```bash
# Run the launcher with dev mode
mcpelauncher-ui-qt -d

# Or if not installed to PATH
~/mcpelauncher-build/mcpelauncher-ui-manifest/build/mcpelauncher-ui-qt -d
```

### Expected Output

If the patch is working correctly, you should NOT see:
```
dlopen failed: cannot locate symbol '__system_property_foreach'
```

Instead, Minecraft 1.21.111+ should load and proceed to initialization.

---

## Step 6: Create Flatpak (Optional)

For a portable installation:

```bash
# Install flatpak-builder
sudo apt install flatpak-builder

# Clone flathub manifest
git clone https://github.com/flathub/io.mrarm.mcpelauncher.git
cd io.mrarm.mcpelauncher

# Modify manifest to use patched source
# (edit io.mrarm.mcpelauncher.json to point to your patched repos)

# Build
flatpak-builder --force-clean build-dir io.mrarm.mcpelauncher.json

# Create bundle
flatpak build-bundle build-dir/repo mcpelauncher-patched.flatpak io.mrarm.mcpelauncher
```

---

## Troubleshooting

### Crash after applying patch

If still crashing, check for other missing issues:

1. **JNI symbols**: May need updates to `jni_support.cpp`
2. **vtable patches**: May need updates to `core_patches.cpp`
3. **FMOD audio**: Ensure libfmod.so.12 is present

### Build errors

```bash
# If CMake can't find Qt6
export Qt6_DIR=/usr/lib/cmake/Qt6

# If missing protobuf
sudo apt install libprotobuf-dev protobuf-compiler
```

### Runtime errors

```bash
# Missing libraries - check with ldd
ldd build/mcpelauncher-client

# Missing FMOD
export LD_LIBRARY_PATH=/path/to/fmod/lib:$LD_LIBRARY_PATH
```

---

## Additional Notes

### Why This Works

The PairIP Core library (`libpairipcore.so`) in Minecraft 1.21.111+ calls Android system property functions to verify device integrity. By providing a proper implementation of these functions with realistic property values, we satisfy the verification while running legitimately on Linux.

### Legal Notice

This patch is intended for users who have legitimately purchased Minecraft Bedrock Edition from the Google Play Store. The implementation provides Android compatibility APIs to allow the game to run on Linux - it does not bypass any licensing or copy protection.

### Contributing

If you find issues or improvements, please contribute back to:
- https://github.com/minecraft-linux/libc-shim
- https://github.com/minecraft-linux/mcpelauncher-manifest
