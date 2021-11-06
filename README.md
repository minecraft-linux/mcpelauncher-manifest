# mcpelauncher-manifest
# This fork is unmaintained since Summer 2020
## See https://github.com/minecraft-linux/mcpelauncher-manifest for a version which runs 1.17+ x86, x86_64, armv7 and arm64 instead of only 0.12 - 1.16.10 x86, armv7
The libhybris library broke with Minecraft 1.16.20 and caused crashs, but the rewritten launcher of minecraft-linux didn't had such problems.

An unstable and dirty fork of the Linux and Mac OS Minecraft Bedrock Edition launcher **client** that supports 1.11+ (incl. most beta builds, 1.0-1.10 cannot be resized after start)

<p align="center">
  <img src="https://user-images.githubusercontent.com/24195572/71060819-64f78c80-211b-11ea-8ea4-f1c2dabb9b54.png"/>
</p>

# Features
- Random unknown crashes
  - provide crashlog, compilerversion and mention the specfic download artifact (Filename, download link)
- Linux x86, amd64 and arm support
- Limited macOS Sierra, High Sierra, Mojave support
  - **reduced gpu performance** [ANGLE - Almost Native Graphics Layer Engine](https://github.com/minecraft-linux/angle) workaround to replace the crashing patch from minecraft-linux
    - render delay while joining the world or server with a lot of objects
    - mobs aren't rendered correctly (known as white mobs (sheeps, spiders...))
      - workaround set initial window size to display resolution
- Support for new Minecraft Releases (1.13+) out of Box
  - The **discontinued mcpelauncher-server** only supports 1.12.0.28 and 1.12.1.1
  - Might break sometime in the future
- **unstable** Xbox live login
  - Sometime fails silently without an errormessage (might be in the log)
    - to try it again close Minecraft and open it again
    - may caused by network problems (e.g. unstable internet access)
    - You need to have a gamertag on your account
  - Friend joining
    - Sometimes only successful during world loading or won't succeed
- less game patches needed than the original launcher
  - defaults to pocket UI
    - e.g. change it in Settings->General->Video->UI Profile from Pocket to Classic
  - defaults to 60 fps vsync
    - vsync is ignored under macOS
    - can be disabled in options.txt
  - defaults to larger buttons
    - e.g. change it in Settings->General->Video->GUI Scale Modifier to -1
  - **Scripting support isn't enabled**, android defaults to no scripting support
    - might added back in the future
  - Initializes the game mostly like android does it
    - results in less function patches
    - implemented "java" functions in c++ rather than patching jni function calls
- Experimental (Chinese, Japanese) IME Support with eglut (not GLFW)
  - **Now have to explicitly to be enabled with**
    - `MCPELAUNCHER_FEATURE_IME=1`
    - to many distros have issues, with x11 ime or xim
  - type Chinese / Japanese with IME while a Textbox is open
    - You need to manually click the Textbox to allow IME to open
    - Only ibus IME is known to work
      - ibus-pinyin
      - ibus-anthy

# Known Issues / Bugs
[Game Launcher](https://github.com/ChristopherHX/mcpelauncher-manifest/issues)
[Game Downloader / Starter](https://github.com/ChristopherHX/mcpelauncher-ui-manifest/issues)
[Xbox Live](https://github.com/ChristopherHX/msa-manifest/issues)

# Installation Guide

## GNU/Linux downloads
https://github.com/ChristopherHX/linux-packaging-scripts/releases

## macOS 10.10 - 10.14, (10.15+ experimental ng) downloads
https://github.com/ChristopherHX/mcpelauncher-manifest/releases

## Build from Source
### Debian
#### x86, x86_64, AMD64, x64
```
# Install dependencies
sudo dpkg --add-architecture i386
sudo apt-get install cmake g++-multilib libpng-dev:i386 libx11-dev:i386 libxi-dev:i386 libcurl4-openssl-dev:i386 libudev-dev:i386 libevdev-dev:i386 libegl1-mesa-dev:i386 libasound2:i386
git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher || cd mcpelauncher && git pull
cd mcpelauncher
mkdir build
cd build
cmake ..
make -j12
```
##### Usage
**Before starting the new launcher, please remember to download the game with the Bedrock launcher QT GUI!**
replace `1.13.X` with your 1.13.X or 1.14.X installation
```
~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X
```
#### armhf, arm64:
```
# Install dependencies
sudo dpkg --add-architecture armhf && sudo apt-get install python3-pip cmake g++-arm-linux-gnueabihf libpng-dev:armhf libx11-dev:armhf libxi-dev:armhf libcurl4-openssl-dev:armhf libudev-dev:armhf libevdev-dev:armhf libegl1-mesa-dev:armhf libasound2:armhf && pip3 install jinja2 ds_store ply
git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher || cd mcpelauncher && git pull
cd mcpelauncher
cd mcpelauncher-linux-bin
git checkout armhf
cd ../minecraft-symbols/tools
python3 ./process_headers.py --armhf
cd ../..
mkdir build
cd build
CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ cmake ..
make -j12
```
##### Usage
**Before starting the new launcher, please remember to download the game with the Bedrock launcher QT GUI!**
replace `1.13.X` with your 1.13.X or 1.14.X installation
```
OPENSSL_armcap=0 ~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X
```

### Archlinux
- prerequisites
  - Add the [multilib Repository](https://wiki.archlinux.org/index.php/Official_repositories#multilib) One time setup (Manjaro has it enabled by default, `sudo bash -c "echo '[multilib]' >> /etc/pacman.conf && echo 'Include = /etc/pacman.d/mirrorlist' >> /etc/pacman.conf"`)
```
# Install dependencies
sudo pacman -Syu --noconfirm git util-linux sudo fakeroot binutils autoconf automake libtool pkgconf file make
git clone https://aur.archlinux.org/lib32-check.git lib32-check || cd lib32-check && git pull
cd lib32-check 
makepkg --syncdeps --noconfirm --install
cd ..
git clone https://aur.archlinux.org/lib32-libevdev.git lib32-libevdev || cd lib32-libevdev && git pull
cd lib32-libevdev
makepkg --syncdeps --noconfirm --install
cd ..
# compile and install the core launcher
git clone https://github.com/ChristopherHX/mcpelauncher-linux-git.git mcpelauncher-linux-git || cd mcpelauncher-linux-git && git pull
cd mcpelauncher-linux-git
makepkg --syncdeps --noconfirm --install
cd ..
# compile and install Xbox live login support
git clone https://aur.archlinux.org/mcpelauncher-msa-git.git mcpelauncher-msa-git || cd mcpelauncher-msa-git && git pull
cd mcpelauncher-msa-git && makepkg --syncdeps --noconfirm --install
git clone https://aur.archlinux.org/mcpelauncher-msa-ui-qt-git.git mcpelauncher-msa-ui-qt-git || cd mcpelauncher-msa-ui-qt-git && git pull
cd mcpelauncher-msa-ui-qt-git
makepkg --syncdeps --noconfirm --install
cd ..
# compile and install the launcher GUI
git clone https://github.com/ChristopherHX/mcpelauncher-ui-git.git mcpelauncher-ui-git || cd mcpelauncher-ui-git && git pull
cd mcpelauncher-ui-git
makepkg --syncdeps --noconfirm --install
cd ..
```
##### Usage
**Before starting the new launcher, please remember to download the game with the Bedrock launcher QT GUI!**
replace `1.13.X` with your 1.13.X or 1.14.X installation
```
mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X
```
or run the launcher GUI
```
mcpelauncher-ui-qt
```
or launch `Minecraft Bedrock Launcher` from start
### Solus
```
# optional keep user folder clean
mkdir -p mcpelauncher && cd mcpelauncher
# Install dependencies
sudo eopkg install -c system.devel
sudo eopkg install git mesalib-32bit-devel openssl-32bit-devel curl-32bit-devel libpng-32bit-devel libx11-32bit-devel gcc g++ libgcc-32bit libstdc++-32bit cmake make binutils glibc-32bit-devel libgudev-32bit-devel libxi-32bit-devel
# Install libevdev-32bit-devel manually
git clone https://gitlab.freedesktop.org/libevdev/libevdev.git || cd libevdev && git pull
cd libevdev
./autogen.sh CFLAGS=-m32
make -j8
sudo make install
ln -s /usr/local/lib/libevdev.so.2 /usr/lib32/libevdev.so.2 || :
cd ..
# Install mcpelauncher-manifest
git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher || cd mcpelauncher && git pull
cd mcpelauncher
mkdir build
cd build
cmake .. -DCMAKE_ASM_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32
make
sudo make install
cd ../..
```
##### Usage
**Before starting the new launcher, please remember to download the game with the Bedrock launcher QT GUI!**
replace `1.13.X` with your 1.13.X or 1.14.X installation
```
mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X
```
