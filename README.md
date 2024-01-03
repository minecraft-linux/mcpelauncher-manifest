# mcpelauncher

An unstable unofficial launcher for the Android version of Minecraft: Bedrock Edition on linux and macOS.

# UI

The User Interface to login, download and manage Versions can be found [here](https://github.com/minecraft-linux/mcpelauncher-ui-manifest)

# System Requirements

Operating System|Architecture|Requirements|Minecraft Versions
---|---|---|---
linux glibc|x86|Intel or Amd CPU with SSSE3 Instruction Set and OpenGL ES 2.0 or newer.|1.13.0 - 1.20.10
linux glibc|x86|Intel or Amd CPU with SSSE3 Instruction Set and OpenGL ES 3.0 or newer.|1.13.0 - 1.20.51
macOS 10.10 - 10.14|x86|Intel or Amd CPU with SSSE3 Instruction Set.|1.13.0 - 1.16.1X
linux glibc|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2,  POPCNT Instruction Sets and OpenGL ES 2.0 or newer.|1.13.0 - 1.20.10
linux glibc|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2, POPCNT Instruction Sets and OpenGL ES 3.0 or newer.|1.13.0 - 1.20.51
macOS 10.10 - latest|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2 and POPCNT Instruction Sets. Rosetta 2 worked in macOS 11 and 12, but no longer in macOS 13|1.13.0 - 1.20.51
linux glibc|arm32|armv7 CPU, pulseaudio and OpenGL ES 2.0 or newer|1.13.0 - 1.18.10
linux glibc|arm64|armv7 CPU, pulseaudio and OpenGL ES 3.0 or newer|1.13.0 - 1.20.51
linux glibc|arm64|armv8 CPU, pulseaudio and OpenGL ES 2.0 or newer|1.13.0 - 1.20.10
linux glibc|arm64|armv8 CPU, pulseaudio and OpenGL ES 3.0 or newer|1.13.0 - 1.20.51
macOS 11.0 - latest|arm64|apple m series CPU (Experimental, modding limitations)|1.19.70 - 1.20.51

# Important Minecraft Version specific Bugs

Minecraft Version|Bug Description
---|---
1.13.0 - 1.16.1X|Xbox login doesn't work. It is working in previous Versions of this Launcher.
1.16.20 - latest|No sound for macOS x86 launcher without compiling yourself with pulseaudio
1.16.210 - 1.17.4X|Crafting is broken on x86 and x86_64 based devices
1.16.210 - 1.17.4X|Textures are off by one on x86 and x86_64 based devices, enable texture patch in profile settings to mitigate this Bug
1.16.210 - 1.17.4X|World Corruption while next to water
1.18.30 - latest|The armv7 port might not work correctly
1.19.50 - latest|You cannot sprint while diagonal strafing on Intel cpu (see [MCPE-170810](https://bugs.mojang.com/browse/MCPE-170810))
all|You are unable to receive Xbox live game invites
all|You may be unable to join worlds hosted by this launcher using Xbox live friends joining

# Wiki

Please check our wiki https://mcpelauncher.readthedocs.io for information about compiling or installing this launcher.

# Nightly Prebuilds

## AppImages and DMG Files

https://github.com/minecraft-linux/mcpelauncher-manifest/releases/tag/nightly

## Debian, Ubuntu and Fedora Packages

https://github.com/minecraft-linux/pkg
