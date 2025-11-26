# MCPELauncher Steam Deck Quick Start Guide

## Quick Installation (Recommended)

### Option 1: Install from Flathub Beta (5 minutes)

On your Steam Deck in Desktop Mode, open Konsole and run:

```bash
# Add Flathub Beta repository
flatpak remote-add --if-not-exists --user flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo

# Install the launcher
flatpak install -y --user flathub-beta io.mrarm.mcpelauncher

# Run with all versions enabled
flatpak run io.mrarm.mcpelauncher -d
```

### Option 2: Use the Setup Script

```bash
# Clone or copy this repository to your Steam Deck
cd steamdeck-build

# Run the setup script
./steamdeck-setup.sh
```

---

## Enabling All Minecraft Versions

### Method 1: Command Line Flag (Per Session)

Always run the launcher with `-d` flag:

```bash
flatpak run io.mrarm.mcpelauncher -d
```

### Method 2: Permanent Configuration

After running with `-d`, go to:
1. **Settings** > **Dev** tab
2. Enable **Show Unverified Versions**
3. Enable **Show Incompatible Versions**
4. Enable **Show Beta Versions**

These settings will persist across sessions.

---

## Adding to Steam Gaming Mode

1. Open Steam in Desktop Mode
2. Click **Games** > **Add a Non-Steam Game**
3. Click **Browse** and navigate to `/usr/bin/flatpak`
4. Add it, then right-click the new entry and select **Properties**
5. Set the name to: `Minecraft Bedrock`
6. Set launch options to: `run io.mrarm.mcpelauncher -d %command%`

---

## Downloading the Latest Minecraft Version

1. Launch the app with `-d` flag
2. Sign in with your Google account (you must own Minecraft on Google Play)
3. Go to **Settings** > **Dev** tab
4. Enable all three version toggles
5. Return to the main screen
6. The latest version should now appear in the download list
7. Click download and wait for completion

---

## Troubleshooting

### Crash on Minecraft 1.21.60+

**Cause**: DNS blocking `vortex.data.microsoft.com`

**Fix**: Ensure this domain is not blocked by your DNS provider or router.

### No Sound

**Fix**: Add to launch options:
```
PULSE_LATENCY_MSEC=60
```

### Graphics Issues / Crash on Start

**Fix**: Try software rendering:
```bash
LIBGL_ALWAYS_SOFTWARE=1 flatpak run io.mrarm.mcpelauncher -d
```

### Version Shows as "Unsupported"

**Fix**:
1. Launch with `-d` flag
2. Go to Settings > Dev tab
3. Enable "Show Incompatible Versions"

### Can't Join Multiplayer Servers

**Fix**: Ensure you have the same Minecraft version as the server. For the latest version, you may need to wait for a launcher update.

---

## Version Compatibility Table

| Minecraft Version | Status | Notes |
|-------------------|--------|-------|
| 1.21.73 | Working | Latest fully supported |
| 1.21.80 - 1.21.92 | Partial | May crash, use nightly builds |
| 1.21.100+ | Experimental | Requires latest nightly |
| 1.20.31+ | Requires OpenGL ES 3.0 | Steam Deck supports this |
| 1.13.0 - 1.20.30 | Full Support | May need older launcher |

---

## Files Created

| File | Purpose |
|------|---------|
| `ANALYSIS.md` | Detailed technical analysis |
| `QUICKSTART.md` | This quick start guide |
| `build-flatpak.sh` | Build script for offline Flatpak |
| `steamdeck-setup.sh` | Configuration script for Steam Deck |
| `io.mrarm.mcpelauncher.steamdeck.json` | Modified Flatpak manifest |
| `patches/enable-all-versions.patch` | Source code patch |

---

## Useful Links

- [MCPELauncher GitHub](https://github.com/minecraft-linux/mcpelauncher-manifest)
- [Official Wiki](https://minecraft-linux.github.io/)
- [Flathub Page](https://flathub.org/en/apps/io.mrarm.mcpelauncher)
- [Issue Tracker](https://github.com/minecraft-linux/mcpelauncher-manifest/issues)
