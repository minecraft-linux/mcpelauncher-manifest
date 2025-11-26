#!/bin/bash
#
# MCPELauncher Steam Deck Setup Script
# Run this on your Steam Deck to configure the launcher for optimal gameplay
#

set -e

echo "================================================"
echo "MCPELauncher Steam Deck Configuration"
echo "================================================"
echo ""

# Create config directory
CONFIG_DIR="${HOME}/.var/app/io.mrarm.mcpelauncher/config"
DATA_DIR="${HOME}/.var/app/io.mrarm.mcpelauncher/data"
mkdir -p "${CONFIG_DIR}"
mkdir -p "${DATA_DIR}"

# Check if launcher is installed
if ! flatpak list --user | grep -q "io.mrarm.mcpelauncher"; then
    echo "MCPELauncher is not installed. Installing from Flathub Beta..."
    flatpak remote-add --if-not-exists --user flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo
    flatpak install -y --user flathub-beta io.mrarm.mcpelauncher
fi

# Create environment configuration
echo "Creating environment configuration..."
cat > "${CONFIG_DIR}/environment.conf" << 'EOF'
# MCPELauncher Steam Deck Environment Configuration
# Created by steamdeck-setup.sh

# Display settings
SDL_VIDEODRIVER=x11

# Audio latency fix
PULSE_LATENCY_MSEC=60

# Mesa/OpenGL settings for Steam Deck AMD GPU
MESA_GL_VERSION_OVERRIDE=4.6
MESA_GLSL_VERSION_OVERRIDE=460

# Force OpenGL ES 3.0 (required for Minecraft 1.20.31+)
# Uncomment if you have graphics issues:
# LIBGL_ALWAYS_SOFTWARE=1

# DNS workaround for version 1.21.60+ crash
# Ensure vortex.data.microsoft.com is accessible
EOF

# Create launcher settings with all versions enabled
echo "Enabling all Minecraft versions..."
SETTINGS_FILE="${DATA_DIR}/GameLauncher/settings.ini"
mkdir -p "$(dirname "${SETTINGS_FILE}")"

# Write settings that enable all versions by default
cat > "${SETTINGS_FILE}" << 'EOF'
[General]
showBetaVersions=true
showUnverified=true
showUnsupported=true
EOF

# Create desktop shortcut for Gaming Mode
echo "Creating Gaming Mode shortcut..."
DESKTOP_FILE="${HOME}/.local/share/applications/mcpelauncher-devmode.desktop"
cat > "${DESKTOP_FILE}" << 'EOF'
[Desktop Entry]
Type=Application
Name=Minecraft Bedrock (All Versions)
Comment=Minecraft Bedrock Edition with all versions enabled
Exec=flatpak run io.mrarm.mcpelauncher -d
Icon=io.mrarm.mcpelauncher
Terminal=false
Categories=Game;
Keywords=minecraft;bedrock;mcpe;
EOF

# Update desktop database
update-desktop-database "${HOME}/.local/share/applications" 2>/dev/null || true

echo ""
echo "================================================"
echo "SETUP COMPLETE!"
echo "================================================"
echo ""
echo "Configuration files created:"
echo "  - ${CONFIG_DIR}/environment.conf"
echo "  - ${SETTINGS_FILE}"
echo "  - ${DESKTOP_FILE}"
echo ""
echo "To launch with all versions enabled:"
echo "  1. From Desktop Mode: Use 'Minecraft Bedrock (All Versions)' app"
echo "  2. From Konsole: flatpak run io.mrarm.mcpelauncher -d"
echo ""
echo "To add to Steam for Gaming Mode:"
echo "  1. Open Steam in Desktop Mode"
echo "  2. Click 'Add a Game' > 'Add a Non-Steam Game'"
echo "  3. Browse to: /usr/bin/flatpak"
echo "  4. Set launch options: run io.mrarm.mcpelauncher -d %command%"
echo ""
echo "IMPORTANT: For Minecraft versions 1.21.60+, ensure:"
echo "  - vortex.data.microsoft.com is NOT blocked by DNS/firewall"
echo "  - Your network connection is stable"
echo ""
echo "For the latest Minecraft version, you may need to:"
echo "  1. Go to Settings > Dev tab (now visible)"
echo "  2. Enable 'Show Unverified Versions'"
echo "  3. Enable 'Show Incompatible Versions' if needed"
echo "  4. Download the desired version"
echo ""
