# mcpelauncher-manifest
An unstable and dirty fork of the Linux and Mac OS Minecraft Bedrock Edition launcher **client** that supports 1.11+ (incl. most beta builds, 1.0-1.10 cannot be resized after start)

 <p align="center">
  <img src="https://user-images.githubusercontent.com/24195572/71060819-64f78c80-211b-11ea-8ea4-f1c2dabb9b54.png"/>
</p>

# Features
- Random unknown crashes
  - provide crashlog and compilerversion (or mention the specfic download artifact)
- Full Linux x86, amd64 and **arm** support
- Limited macOS Sierra, High Sierra, Mojave support
  - **reduced gpu performance** uses a patched [ANGLE - Almost Native Graphics Layer Engine](https://github.com/minecraft-linux/angle) to replace the crashing patch from minecraft-linux
    - render delay while joining the world or server with a lot of objects
    - mobs aren't rendered correctly (known as white mobs (sheeps, spiders...))
      - workaround set initial window size to display resolution
- Support for new Minecraft Releases (1.13+) out of Box
  - **The mcpelauncher-server only supports 1.12.0.28 and 1.12.1.1**
  - Might break sometime in the future
- **unstable** Xbox live login
  - Sometime fails silently without an errormessage (might be in the log)
    - to try it again close Minecraft and open it again
    - may caused by network problems (e.g. unstable internet access)
    - now crashes after trying starting msa multiple times without success
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
- More prebuild Linux (disribution) packages supported than the original launcher
  - **Pure Linux x86 distro support on old PCs (SSSE3 CPU instruction set needed)**
  - **Linux ARM appimage**
    - experimental and was only tested on raspbian
- Experimental (Chinese, Japanese) IME Support with eglut (not GLFW)
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
**Currently are the mcpelauncher-client and mcpelauncher-server &ast;.deb packages incompatible**

## AppImage (with update indicator)
and Packages for Debian, Ubuntu, archlinux and manjarolinux
https://github.com/ChristopherHX/linux-packaging-scripts/releases

## Install Prebuilds for MacOs 10.12 - 10.14
https://github.com/ChristopherHX/mcpelauncher-manifest/releases

## Build from Source
### Debian
- First, make sure you have all needed dependencies (drivers) installed:
- x86, x86_64, AMD64, x64:

  `sudo dpkg --add-architecture i386 && sudo apt-get install cmake g++-multilib libpng-dev:i386 libx11-dev:i386 libxi-dev:i386 libcurl4-openssl-dev:i386 libudev-dev:i386 libevdev-dev:i386 libegl1-mesa-dev:i386 libasound2:i386`
- armhf, arm64:

  `sudo dpkg --add-architecture armhf && sudo apt-get install python3-pip cmake g++-arm-linux-gnueabihf libpng-dev:armhf libx11-dev:armhf libxi-dev:armhf libcurl4-openssl-dev:armhf libudev-dev:armhf libevdev-dev:armhf libegl1-mesa-dev:armhf libasound2:armhf && pip3 install jinja2 ds_store ply`
  - Only Raspberry Pi (Note: raspbian buster with Pi4 and beyond, you might need to override preinstalled mesa)
    - Download https://github.com/ChristopherHX/mcpelauncher-manifest/releases/download/1.12.x.2.armhf.raspbian.buster/mesa.tar.xz and extract to $HOME/mesa
    - it should contain `$HOME/mesa/include` and `$HOME/mesa/lib`
- Next, in the same window, run `git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher && cd mcpelauncher` in the home directory.
- Also run the following commands below:
- `mkdir -p build && cd build`
- x86, x86_64, AMD64, x64:

  `cmake .. && make -j12`
- armhf, arm64:
  
    `cd ../mcpelauncher-linux-bin && git checkout armhf && cd ../minecraft-symbols/tools && python3 ./process_headers.py --armhf && cd ../../build`
  - non Raspberry Pi
    
    `CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ cmake -DCMAKE_CXX_FLAGS="-latomic" .. && make -j12`
  - Raspberry Pi

    `CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ cmake -DCMAKE_CXX_FLAGS="-isystem $HOME/mesa/include -latomic" .. && make`

- Finally
  - x86, x86_64, AMD64, x64:
  
    `~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - armhf, arm64 (non Raspberry Pi):

    `OPENSSL_armcap=0 ~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - armhf, arm64 Raspberry Pi:
    - Warning only mesa 13.x from https://www.mesa3d.org/ seems to avoid huge rendering glitches
    - But will cause a Blackscreen if another mesa version is used, this affects the Pi4!
    `LD_LIBRARY_PATH=$HOME/mesa/lib LIBGL_DRIVERS_PATH=$HOME/mesa/lib/dri GBM_DRIVERS_PATH=$HOME/mesa/lib OPENSSL_armcap=0 ~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - Then Minecraft Bedrock 1.13.x will begin to start up. Make sure you replace `1.13.X` with your 1.13.X or 1.14.X installation, otherwise this setup will not work for you

**Before starting the new launcher, please remember to download the game with the original Bedrock launcher QT GUI!**

### Archlinux
- prerequisites
  - Add the [multilib Repository](https://wiki.archlinux.org/index.php/Official_repositories#multilib) One time setup (Manjaro has it enabled by default, `sudo bash -c "echo '[multilib]' >> /etc/pacman.conf && echo 'Include = /etc/pacman.d/mirrorlist' >> /etc/pacman.conf"`)

  `sudo pacman -Syu --noconfirm git util-linux sudo fakeroot binutils autoconf automake libtool pkgconf file make`

  `(git clone https://github.com/ChristopherHX/lib32-check.git lib32-check || cd lib32-check && git pull) && cd lib32-check && makepkg --syncdeps --noconfirm --install && cd .. && (git clone https://aur.archlinux.org/lib32-libevdev.git lib32-libevdev && cd lib32-libevdev || cd lib32-libevdev && git pull) && cd lib32-libevdev && makepkg --syncdeps --noconfirm --install || :`

- compile and install the core launcher

  `(git clone https://github.com/ChristopherHX/mcpelauncher-linux-git.git mcpelauncher-linux-git || cd mcpelauncher-linux-git && git pull) && cd mcpelauncher-linux-git && makepkg --syncdeps --noconfirm --install || :`

- compile and install Xbox live login support

  `((git clone https://aur.archlinux.org/mcpelauncher-msa-git.git mcpelauncher-msa-git || cd mcpelauncher-msa-git && git pull)  && cd mcpelauncher-msa-git && makepkg --syncdeps --noconfirm --install || :) && ((git clone https://aur.archlinux.org/mcpelauncher-msa-ui-qt-git.git mcpelauncher-msa-ui-qt-git || cd mcpelauncher-msa-ui-qt-git && git pull) && cd mcpelauncher-msa-ui-qt-git && makepkg --syncdeps --noconfirm --install || :)`


- compile and install the launcher GUI

  `(git clone https://github.com/ChristopherHX/mcpelauncher-ui-git.git mcpelauncher-ui-git || cd mcpelauncher-ui-git && git pull) && cd mcpelauncher-ui-git && makepkg --syncdeps --noconfirm --install || :`
  
### Solus
- optional `mkdir -p mcpelauncher && cd mcpelauncher`
  - keeps user folder clean
- `sudo eopkg install -c system.devel`
- `sudo eopkg install git mesalib-32bit-devel openssl-32bit-devel curl-32bit-devel libpng-32bit-devel libx11-32bit-devel gcc g++ libgcc-32bit libstdc++-32bit cmake make binutils glibc-32bit-devel libgudev-32bit-devel libxi-32bit-devel`
- Install libevdev-32bit-devel manually
`(git clone https://gitlab.freedesktop.org/libevdev/libevdev.git || cd libevdev && git pull) && cd libevdev && ./autogen.sh CFLAGS=-m32 && make && sudo make install && ln -s /usr/local/lib/libevdev.so.2 /usr/lib32/libevdev.so.2 || :`
- Install mcpelauncher-manifest
`(git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher || cd mcpelauncher && git pull) && cd mcpelauncher && mkdir -p build && cd build && cmake .. -DCMAKE_ASM_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 && make && sudo make install || :`
