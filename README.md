# mcpelauncher

An unstable unofficial launcher for the Android version of Minecraft: Bedrock Edition on linux and macOS.

# UI

The User Interface to login, download and manage Versions can be found [here](https://github.com/minecraft-linux/mcpelauncher-ui-manifest)

# System Requirements

Operating System|Architecture|Requirements|Minecraft Versions
---|---|---|---
linux glibc|x86|Intel or Amd CPU with SSSE3 Instruction Set.|1.13.0 - 1.19.81
macOS 10.10 - 10.14|x86|Intel or Amd CPU with SSSE3 Instruction Set.|1.13.0 - 1.16.1X
linux glibc|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2 and POPCNT Instruction Sets.|1.13.0 - 1.19.81
macOS 10.10 - latest|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2 and POPCNT Instruction Sets. M1 Macs can run this Launcher with Rosetta 2|1.13.0 - 1.19.81
linux glibc|arm32|armv7 CPU and pulseaudio|1.13.0 - 1.19.81
linux glibc|arm64|armv8 CPU and pulseaudio|1.13.0 - 1.19.81


# Important Minecraft Version specific Bugs

Minecraft Version|Bug Description
---|---
1.13.0 - 1.16.1X|Xbox login doesn't work. It is working in previous Versions of this Launcher.
1.16.20 - latest|No sound for macOS x86 launcher without compiling yourself with pulseaudio
1.16.210 - 1.17.4X|Crafting is broken on x86 and x86_64 based devices
1.16.210 - 1.17.4X|Textures are off by one on x86 and x86_64 based devices, enable texture patch in profile settings to mitigate this Bug
1.16.210 - 1.17.4X|World Corruption while next to water
all|You are unable to receive Xbox live game invites
all|You may be unable to join worlds hosted by this laucher using Xbox live friends joining

# Wiki

Please check our wiki https://mcpelauncher.readthedocs.io for information about compiling or installing this launcher.
