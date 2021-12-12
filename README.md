# mcpelauncher

This project contains an unofficial launcher for the Android version of Minecraft: Bedrock Edition on linux and macOS.

Currently this Launcher can open Minecraft 1.13.0 - 1.18.2. Previous Versions of this Launcher were able to run 0.12 - 1.12.1.

# System Requirements
Your Computer must be able to run Android.

Operating System|Architecture|Requirements
---|---|---
linux / macOS 10.10 - 10.14|x86|Intel or Amd CPU with SSSE3 Instruction Set.
linux / macOS 10.10 - latest|x86_64|Intel or Amd CPU with SSSE3, SSE4.1, SSE4.2 and POPCNT Instruction Sets. M1 Macs can run this Launcher with Rosetta 2
linux|arm32|armv7 CPU and pulseaudio
linux|arm64|armv8 CPU and pulseaudio


# Notable Version specfic Bug
Minecraft Version|Bug Description
---|---
1.13.0 - 1.16.1X|Xbox login doesn't work. It is working in previous Versions of this Launcher.
1.16.20 - latest|No sound for macOS x86 launcher without compiling yourself with pulseaudio
1.16.100 - 1.17.2X|Pressing the archivement button crashs the game
1.16.210 - 1.17.4X|Crafting not working as expected
1.16.210 - 1.17.4X|Textures off by one, enable texture patch in profile settings to mitigate this Bug
1.16.210 - 1.17.4X|World Corruption while next to water
1.16.210 - latest|Store and skins not loading
1.17.4X - latest|Pressing the archivement button does nothing

# Wiki
Please check our wiki https://mcpelauncher.readthedocs.io for information about compiling or installing this launcher.