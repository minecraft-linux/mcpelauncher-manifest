# mcpelauncher-manifest
A fork of MCPE Launcher created by ChristopherHX for the Linux and Mac OS Bedrock Edition Minecraft launcher that supports 1.13.x and 1.14.x.

# Features
- Xbox Authentication [*], **WORKS ONLY ON x86, x86_64, AMD64, x64, Currently no Autologin (login each time starting the game!)**
- **ARM currently no Mouse capture support**

# Installation Guide
## Install Prebuilds for Debian / Ubuntu / archlinux / manjarolinux
https://github.com/ChristopherHX/linux-packaging-scripts/releases

If you have the minecraft-linux launcher already installed then you need only to install the mcpelauncher-&ast;-client.deb (Debian / Ubuntu) or mcpelauncher-linux-git-&ast;.pkg.tar.xz (archlinux / manjarolinux) (No need to uninstall anything)

### Debian / Ubuntu
Install it via `sudo dpkg -i *.deb && sudo apt-get install -f` or using the Userinterface

### archlinux / manjarolinux
Install it via `sudo pacman -U *.pkg.tar.xz` or use the custom AUR packages (Sourcebuild)

### Appimage minimal dependencies - Gamelauncher not found (Debian (ChromeOS Linux) / Ubuntu / AMD64)
- `sudo dpkg --add-architecture i386 && sudo apt-get update && sudo apt install libc6-i386 libegl1-mesa:i386 zlib1g:i386 libstdc++6:i386 libgl1-mesa-dri:i386`
## Install Prebuilds for MacOs 10.12 - 10.14
https://github.com/ChristopherHX/mcpelauncher-manifest/releases

### Experimental Builds (Release Page maybe outdated)
- Goto https://github.com/ChristopherHX/mcpelauncher-manifest/commits/master
- Then click the green checkmark of the latest commit, click Details (popup)
- Upper right under Artifacts is the download DMG in a .zip

#### If the Minecraft game not fit into the Window
**Maybe you see only the lower left quater of the Game then try this**
- Goto https://github.com/ChristopherHX/mcpelauncher-manifest/commits/sizebug
- Then click the green checkmark of the latest commit, click Details (popup)
- Upper right under Artifacts is the download DMG in a .zip

## Build from Source
**gcc < 8.3.0 generates unusable launchers**
### Debian based (apt, dpkg, .deb)
- First, make sure you have all needed dependencies (drivers) installed:
- x86, x86_64, AMD64, x64: `sudo dpkg --add-architecture i386 && sudo apt-get install cmake g++-multilib libpng-dev:i386 libx11-dev:i386 libxi-dev:i386 libcurl4-openssl-dev:i386 libudev-dev:i386 libevdev-dev:i386 libegl1-mesa-dev:i386 libasound2:i386`
- armhf, arm64:
  - `sudo dpkg --add-architecture armhf && sudo apt-get install python3-pip cmake g++-arm-linux-gnueabihf libpng-dev:armhf libx11-dev:armhf libxi-dev:armhf libcurl4-openssl-dev:armhf libudev-dev:armhf libevdev-dev:armhf libegl1-mesa-dev:armhf libasound2:armhf && pip3 install jinja2 ds_store ply`
  - Download https://github.com/ChristopherHX/mcpelauncher-manifest/releases/download/1.12.x.2.armhf.raspbian.buster/mesa.tar.xz and extract to $HOME/mesa
  - it should contain `$HOME/mesa/include` and `$HOME/mesa/lib`
- Next, in the same window, run `git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher && cd mcpelauncher` in the home directory.
- Also run the following commands below:
- `mkdir -p build && cd build`
- x86, x86_64, AMD64, x64:`cmake ..`
- armhf, arm64:
  - `cd ../mcpelauncher-linux-bin && git checkout armhf && cd ../minecraft-symbols/tools && python3 ./process_headers.py --armhf && cd ../../build`
  - `CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ cmake -DCMAKE_CXX_FLAGS="-isystem $HOME/mesa/include -latomic" ..`
- `make -j12`

- Finally
  - ~~`cd ~/.local/share/mcpelauncher/versions/1.13.X/assets`~~ No longer needed
  - x86, x86_64, AMD64, x64: `~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - armhf, arm64: `LD_LIBRARY_PATH=$HOME/mesa/lib LIBGL_DRIVERS_PATH=$HOME/mesa/lib/dri GBM_DRIVERS_PATH=$HOME/mesa/lib OPENSSL_armcap=0 ~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - Then Minecraft Bedrock 1.13.x will begin to start up. Make sure you replace `1.13.X` with your 1.13.X or 1.14.X installation, otherwise this setup will not work for you

**Before starting the new launcher, please remember to download the game with the original Bedrock launcher QT GUI!**
### Archlinux based (pacman)
- Add the [multilib Repository](https://wiki.archlinux.org/index.php/Official_repositories#multilib) One time setup (Manjaro has it enabled by default, `sudo bash -c "echo '[multilib]' >> /etc/pacman.conf && echo 'Include = /etc/pacman.d/mirrorlist' >> /etc/pacman.conf"`)

`sudo pacman -Syu --noconfirm git util-linux sudo fakeroot binutils autoconf automake libtool pkgconf file make`

`git clone https://github.com/ChristopherHX/lib32-check.git lib32-check && cd lib32-check && makepkg --syncdeps --noconfirm --install && cd ..`

`git clone https://aur.archlinux.org/lib32-libevdev.git lib32-libevdev && cd lib32-libevdev && makepkg --syncdeps --noconfirm --install && cd ..`

`git clone https://github.com/ChristopherHX/mcpelauncher-linux-git.git mcpelauncher-linux-git && cd mcpelauncher-linux-git && makepkg --syncdeps --noconfirm --install && cd ..`

`git clone https://aur.archlinux.org/mcpelauncher-msa-git.git mcpelauncher-msa-git && cd mcpelauncher-msa-git && makepkg --syncdeps --noconfirm --install && cd ..`

`git clone https://aur.archlinux.org/mcpelauncher-msa-ui-qt-git.git mcpelauncher-msa-ui-qt-git && cd mcpelauncher-msa-ui-qt-git && makepkg --syncdeps --noconfirm --install && cd ..`

`git clone https://github.com/ChristopherHX/mcpelauncher-ui-git.git mcpelauncher-ui-git && cd mcpelauncher-ui-git && makepkg --syncdeps --noconfirm --install && cd ..`
