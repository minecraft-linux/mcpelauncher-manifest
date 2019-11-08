# mcpelauncher-manifest
A fork of MCPE Launcher created by ChristopherHX for the Linux and Mac OS Bedrock Edition Minecraft launcher that supports 1.13.x and 1.14.x.

# Features
- Character Creator [X]
- Fox [X]
- All MC 1.13 features without crashes [X]
- Xbox Authentication [ ]

# Installation Guide
## Install Prebuilds for Ubuntu
https://github.com/ChristopherHX/linux-packaging-scripts/releases
## Build from Source
- First, make sure you have all needed drivers installed:
`sudo apt-get install g++-multilib libpng-dev:i386 libx11-dev:i386 libxi-dev:i386 libcurl4-openssl-dev:i386 libudev-dev:i386 libevdev-dev:i386 libegl1-mesa-dev:i386 libasound2:i386`
- Next, in the same window, run `git clone --recursive https://github.com/ChristopherHX/mcpelauncher-manifest.git mcpelauncher && cd mcpelauncher`in the home directory.
- Also run the following commands below:
- `mkdir -p build && cd build`
- `cmake ..`
- `make -j12`

- Finally
  - ~~`cd ~/.local/share/mcpelauncher/versions/1.13.X/assets`~~ No longer needed
  - `~/mcpelauncher/build/mcpelauncher-client/mcpelauncher-client -dg ~/.local/share/mcpelauncher/versions/1.13.X`
  - Then Minecraft Bedrock 1.13.x will begin to start up. Make sure you replace `1.13.X` with your 1.13.X or 1.14.X installation, otherwise this setup will not work for you

**Before starting the new launcher, please remember to download the game with the original Bedrock launcher QT GUI!**