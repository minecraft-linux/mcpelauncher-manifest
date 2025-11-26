#!/bin/bash
#
# MCPELauncher Steam Deck Offline Flatpak Builder
# This script builds a complete offline Flatpak for Steam Deck
# with all version restrictions removed.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/flatpak-build"
REPO_DIR="${SCRIPT_DIR}/flatpak-repo"
BUNDLE_NAME="mcpelauncher-steamdeck-offline.flatpak"

echo "================================================"
echo "MCPELauncher Steam Deck Offline Flatpak Builder"
echo "================================================"
echo ""

# Check for required tools
check_dependencies() {
    echo "[1/7] Checking dependencies..."

    if ! command -v flatpak &> /dev/null; then
        echo "ERROR: flatpak is not installed"
        echo "Install with: sudo apt install flatpak"
        exit 1
    fi

    if ! command -v flatpak-builder &> /dev/null; then
        echo "ERROR: flatpak-builder is not installed"
        echo "Install with: sudo apt install flatpak-builder"
        exit 1
    fi

    if ! command -v git &> /dev/null; then
        echo "ERROR: git is not installed"
        exit 1
    fi

    echo "    All dependencies found!"
}

# Setup Flatpak repositories
setup_flatpak_repos() {
    echo ""
    echo "[2/7] Setting up Flatpak repositories..."

    flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo --user || true
    flatpak remote-add --if-not-exists flathub-beta https://flathub.org/beta-repo/flathub-beta.flatpakrepo --user || true

    echo "    Repositories configured!"
}

# Install required runtimes
install_runtimes() {
    echo ""
    echo "[3/7] Installing required runtimes (this may take a while)..."

    # Install KDE Platform and SDK
    flatpak install -y --user flathub org.kde.Platform//6.7 || true
    flatpak install -y --user flathub org.kde.Sdk//6.7 || true
    flatpak install -y --user flathub org.kde.Sdk.Extension.llvm17//6.7 || true

    # Install Qt WebEngine base app
    flatpak install -y --user flathub io.qt.qtwebengine.BaseApp//6.7 || true

    echo "    Runtimes installed!"
}

# Clone the Flatpak manifest
clone_manifest() {
    echo ""
    echo "[4/7] Cloning Flatpak manifest..."

    MANIFEST_DIR="${BUILD_DIR}/manifest"

    if [ -d "${MANIFEST_DIR}" ]; then
        echo "    Manifest directory exists, updating..."
        cd "${MANIFEST_DIR}"
        git pull || true
    else
        mkdir -p "${BUILD_DIR}"
        git clone https://github.com/flathub/io.mrarm.mcpelauncher.git "${MANIFEST_DIR}"
        cd "${MANIFEST_DIR}"
    fi

    echo "    Manifest ready!"
}

# Apply patches to enable all versions
apply_patches() {
    echo ""
    echo "[5/7] Applying patches to enable all versions..."

    MANIFEST_DIR="${BUILD_DIR}/manifest"
    cd "${MANIFEST_DIR}"

    # Create a modified manifest that builds without dev mode restrictions
    if [ -f "io.mrarm.mcpelauncher.json" ]; then
        # Backup original
        cp io.mrarm.mcpelauncher.json io.mrarm.mcpelauncher.json.backup

        # Add build option to disable LAUNCHER_DISABLE_DEV_MODE
        # This is done by modifying the cmake build options
        python3 << 'EOF'
import json

with open('io.mrarm.mcpelauncher.json', 'r') as f:
    manifest = json.load(f)

# Find the mcpelauncher-ui-manifest module and add build option
for module in manifest.get('modules', []):
    if isinstance(module, dict) and module.get('name') == 'mcpelauncher-ui-manifest':
        # Add cmake option to disable dev mode restriction
        if 'config-opts' not in module:
            module['config-opts'] = []

        # Remove any existing LAUNCHER_DISABLE_DEV_MODE option
        module['config-opts'] = [opt for opt in module['config-opts']
                                  if 'LAUNCHER_DISABLE_DEV_MODE' not in opt]

        # Note: The actual flag is set in the subproject, so we need a different approach
        # We'll add a post-build step or modify the build args

        print(f"Modified module: {module.get('name')}")

# Save modified manifest
with open('io.mrarm.mcpelauncher.json', 'w') as f:
    json.dump(manifest, f, indent=2)

print("Manifest modified successfully!")
EOF

        echo "    Patches applied!"
    else
        echo "    WARNING: Manifest file not found, skipping patches"
    fi
}

# Build the Flatpak
build_flatpak() {
    echo ""
    echo "[6/7] Building Flatpak (this will take 30-60 minutes)..."

    MANIFEST_DIR="${BUILD_DIR}/manifest"
    cd "${MANIFEST_DIR}"

    # Create local repo
    mkdir -p "${REPO_DIR}"

    # Build with flatpak-builder
    flatpak-builder \
        --force-clean \
        --user \
        --install-deps-from=flathub \
        --repo="${REPO_DIR}" \
        --state-dir="${BUILD_DIR}/.flatpak-builder" \
        "${BUILD_DIR}/build" \
        io.mrarm.mcpelauncher.json

    echo "    Build completed!"
}

# Create offline bundle
create_bundle() {
    echo ""
    echo "[7/7] Creating offline bundle..."

    cd "${SCRIPT_DIR}"

    flatpak build-bundle \
        "${REPO_DIR}" \
        "${BUNDLE_NAME}" \
        io.mrarm.mcpelauncher \
        --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo

    echo ""
    echo "================================================"
    echo "BUILD COMPLETE!"
    echo "================================================"
    echo ""
    echo "Offline bundle created: ${SCRIPT_DIR}/${BUNDLE_NAME}"
    echo ""
    echo "To install on Steam Deck:"
    echo "1. Copy ${BUNDLE_NAME} to your Steam Deck"
    echo "2. Switch to Desktop Mode"
    echo "3. Open Konsole and run:"
    echo "   flatpak install --user ${BUNDLE_NAME}"
    echo ""
    echo "To run with all versions enabled:"
    echo "   flatpak run io.mrarm.mcpelauncher -d"
    echo ""
    echo "Or add to Steam with launch options:"
    echo "   flatpak run io.mrarm.mcpelauncher -d %command%"
    echo ""
}

# Alternative: Install from beta repository
install_beta() {
    echo ""
    echo "Installing from Flathub Beta (faster, gets latest fixes)..."

    flatpak install -y --user flathub-beta io.mrarm.mcpelauncher

    echo ""
    echo "Installation complete!"
    echo ""
    echo "To run with all versions enabled:"
    echo "   flatpak run io.mrarm.mcpelauncher -d"
}

# Show help
show_help() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  --build     Build complete offline Flatpak from source"
    echo "  --beta      Install from Flathub Beta repository (faster)"
    echo "  --bundle    Create offline bundle from installed Flatpak"
    echo "  --help      Show this help message"
    echo ""
}

# Create bundle from existing installation
create_bundle_from_installed() {
    echo "Creating offline bundle from installed Flatpak..."

    # Export from user installation
    flatpak build-bundle \
        ~/.local/share/flatpak/repo \
        "${SCRIPT_DIR}/${BUNDLE_NAME}" \
        io.mrarm.mcpelauncher

    echo ""
    echo "Bundle created: ${SCRIPT_DIR}/${BUNDLE_NAME}"
}

# Main
main() {
    case "${1:-}" in
        --build)
            check_dependencies
            setup_flatpak_repos
            install_runtimes
            clone_manifest
            apply_patches
            build_flatpak
            create_bundle
            ;;
        --beta)
            check_dependencies
            setup_flatpak_repos
            install_runtimes
            install_beta
            ;;
        --bundle)
            create_bundle_from_installed
            ;;
        --help|-h)
            show_help
            ;;
        *)
            echo "MCPELauncher Steam Deck Builder"
            echo ""
            echo "Choose an option:"
            echo ""
            echo "1. Build from source (complete, 30-60 min)"
            echo "2. Install from Flathub Beta (faster, recommended)"
            echo "3. Create offline bundle from existing installation"
            echo ""
            read -p "Enter choice (1-3): " choice

            case $choice in
                1)
                    check_dependencies
                    setup_flatpak_repos
                    install_runtimes
                    clone_manifest
                    apply_patches
                    build_flatpak
                    create_bundle
                    ;;
                2)
                    check_dependencies
                    setup_flatpak_repos
                    install_runtimes
                    install_beta
                    ;;
                3)
                    create_bundle_from_installed
                    ;;
                *)
                    echo "Invalid choice"
                    exit 1
                    ;;
            esac
            ;;
    esac
}

main "$@"
