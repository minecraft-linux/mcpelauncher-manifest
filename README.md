# mcpelauncher-manifest
A fork of MCPE Launcher created by ChristopherHX for the Linux and Mac OS Bedrock Edition Minecraft launcher that supports 1.13.x and 1.14.x.

# Features
- Xbox Authentication [*], **WORKS ONLY ON x86, x86_64, AMD64, x64, Currently no Autologin (login each time starting the game!)**
- **ARM currently no Mouse capture support**

# Installation Guide
## Install Prebuilds for Debian / Ubuntu / archlinux / manjarolinux
https://github.com/ChristopherHX/linux-packaging-scripts/releases
### Appimage minimal dependences - Gamelauncher not found (Debian (ChromeOS Linux) / Ubuntu / AMD64)
- `sudo dpkg --add-architecture i386 && sudo apt-get update && sudo apt install libc6-i386 libegl1-mesa:i386 zlib1g:i386 libstdc++6:i386 libgl1-mesa-dri:i386`
## Install Prebuilds for MacOs 10.12 - 10.14
**Adding libpng x86 to MacOs isn't easy**
- Goto https://github.com/ChristopherHX/mcpelauncher-manifest/commits/master
- Then click the green checkmark of the latest commit, click Details (popup)
- Upper right under Artifacts is the download DMG in a .zip

### If the Minecraft game not fit into the Window
**Maybe you see only the lower left quater of the Game then try this**
- Goto https://github.com/ChristopherHX/mcpelauncher-manifest/commits/sizebug
- Then click the green checkmark of the latest commit, click Details (popup)
- Upper right under Artifacts is the download DMG in a .zip

## Build from Source (gcc < 8.3.0 generates unusable launchers)
- First, make sure you have all needed drivers installed:
- x86, x86_64, AMD64, x64: `sudo dpkg --add-architecture i386 && sudo apt-get install g++-multilib libpng-dev:i386 libx11-dev:i386 libxi-dev:i386 libcurl4-openssl-dev:i386 libudev-dev:i386 libevdev-dev:i386 libegl1-mesa-dev:i386 libasound2:i386`
- armhf, arm64:
  - `sudo dpkg --add-architecture armhf && sudo apt-get install python3-pip g++-arm-linux-gnueabihf libpng-dev:armhf libx11-dev:armhf libxi-dev:armhf libcurl4-openssl-dev:armhf libudev-dev:armhf libevdev-dev:armhf libegl1-mesa-dev:armhf libasound2:armhf && pip3 install jinja2 ds_store`
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
