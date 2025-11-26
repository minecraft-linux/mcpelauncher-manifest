#!/bin/bash
#
# Minecraft APK Symbol Extraction Tool
# Extracts and analyzes symbols from Minecraft Bedrock APKs
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/output"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

usage() {
    echo "Minecraft APK Symbol Extraction Tool"
    echo ""
    echo "Usage: $0 <apk_file> [architecture]"
    echo ""
    echo "Arguments:"
    echo "  apk_file      Path to Minecraft APK file"
    echo "  architecture  Optional: x86_64, arm64-v8a, armeabi-v7a, x86 (default: x86_64)"
    echo ""
    echo "Examples:"
    echo "  $0 minecraft-1.21.80.apk"
    echo "  $0 minecraft-1.21.120.apk arm64-v8a"
    echo ""
    echo "This script will:"
    echo "  1. Extract the APK"
    echo "  2. Find libminecraftpe.so and libpairipcore.so"
    echo "  3. Dump all symbols using nm and objdump"
    echo "  4. Filter for relevant symbols (MainActivity, AppPlatform, etc.)"
    echo "  5. Create comparison-ready output files"
}

check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"

    local missing=()

    command -v unzip &> /dev/null || missing+=("unzip")
    command -v nm &> /dev/null || missing+=("binutils (nm)")
    command -v objdump &> /dev/null || missing+=("binutils (objdump)")
    command -v readelf &> /dev/null || missing+=("binutils (readelf)")

    if [ ${#missing[@]} -gt 0 ]; then
        echo -e "${RED}Missing dependencies: ${missing[*]}${NC}"
        echo "Install with: sudo apt install unzip binutils"
        exit 1
    fi

    echo -e "${GREEN}All dependencies found!${NC}"
}

extract_apk() {
    local apk_file="$1"
    local extract_dir="$2"

    echo -e "${YELLOW}Extracting APK...${NC}"

    if [ -d "$extract_dir" ]; then
        rm -rf "$extract_dir"
    fi

    mkdir -p "$extract_dir"
    unzip -q "$apk_file" -d "$extract_dir"

    echo -e "${GREEN}APK extracted to: $extract_dir${NC}"
}

find_libraries() {
    local extract_dir="$1"
    local arch="$2"

    echo -e "${YELLOW}Finding libraries for architecture: $arch${NC}"

    local lib_dir="$extract_dir/lib/$arch"

    if [ ! -d "$lib_dir" ]; then
        echo -e "${RED}Library directory not found: $lib_dir${NC}"
        echo "Available architectures:"
        ls -la "$extract_dir/lib/" 2>/dev/null || echo "No lib directory found"
        exit 1
    fi

    echo "Libraries found:"
    ls -la "$lib_dir"/*.so 2>/dev/null || echo "No .so files found"

    echo "$lib_dir"
}

dump_symbols() {
    local lib_file="$1"
    local output_prefix="$2"
    local lib_name="$(basename "$lib_file")"

    echo -e "${YELLOW}Dumping symbols from: $lib_name${NC}"

    # Dynamic symbols (exported)
    echo "  - Extracting dynamic symbols..."
    nm -D "$lib_file" 2>/dev/null | sort > "${output_prefix}_dynamic_symbols.txt" || true

    # All symbols (if available)
    echo "  - Extracting all symbols..."
    nm "$lib_file" 2>/dev/null | sort > "${output_prefix}_all_symbols.txt" || true

    # Symbol table from readelf
    echo "  - Reading ELF symbol table..."
    readelf -s "$lib_file" 2>/dev/null > "${output_prefix}_elf_symbols.txt" || true

    # Exported functions
    echo "  - Extracting exported functions..."
    objdump -T "$lib_file" 2>/dev/null | grep "DF" | awk '{print $NF}' | sort > "${output_prefix}_exports.txt" || true

    echo -e "${GREEN}Symbols dumped for: $lib_name${NC}"
}

filter_relevant_symbols() {
    local symbols_file="$1"
    local output_dir="$2"
    local prefix="$3"

    echo -e "${YELLOW}Filtering relevant symbols...${NC}"

    # MainActivity symbols
    echo "  - MainActivity symbols..."
    grep -i "MainActivity" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_MainActivity.txt" || true

    # AppPlatform symbols
    echo "  - AppPlatform symbols..."
    grep -i "AppPlatform" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_AppPlatform.txt" || true

    # JNI-related symbols
    echo "  - JNI symbols (Java_)..."
    grep "^Java_" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_JNI_methods.txt" || true
    grep " Java_" "$symbols_file" 2>/dev/null >> "${output_dir}/${prefix}_JNI_methods.txt" || true

    # Native methods
    echo "  - Native methods..."
    grep -i "native" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_native.txt" || true

    # vtable symbols
    echo "  - vtable symbols..."
    grep "_ZTV" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_vtables.txt" || true

    # pairipcore-related
    echo "  - pairipcore symbols..."
    grep -i "pairip\|integrity\|property" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_pairipcore.txt" || true

    # System property related
    echo "  - System property symbols..."
    grep -i "system_property\|__system" "$symbols_file" 2>/dev/null > "${output_dir}/${prefix}_system_property.txt" || true

    echo -e "${GREEN}Filtering complete!${NC}"
}

generate_report() {
    local output_dir="$1"
    local version="$2"
    local arch="$3"

    echo -e "${YELLOW}Generating report...${NC}"

    local report_file="${output_dir}/SYMBOL_REPORT.md"

    cat > "$report_file" << EOF
# Minecraft Bedrock Symbol Analysis Report

**Version:** $version
**Architecture:** $arch
**Generated:** $(date)

---

## Summary

### MainActivity Symbols
$(wc -l < "${output_dir}/${version}_MainActivity.txt" 2>/dev/null || echo "0") symbols found

### AppPlatform Symbols
$(wc -l < "${output_dir}/${version}_AppPlatform.txt" 2>/dev/null || echo "0") symbols found

### JNI Methods (Java_*)
$(wc -l < "${output_dir}/${version}_JNI_methods.txt" 2>/dev/null || echo "0") methods found

### vtable Symbols
$(wc -l < "${output_dir}/${version}_vtables.txt" 2>/dev/null || echo "0") vtables found

### System Property Symbols
$(wc -l < "${output_dir}/${version}_system_property.txt" 2>/dev/null || echo "0") symbols found

---

## Key Symbols

### MainActivity JNI Methods
\`\`\`
$(head -50 "${output_dir}/${version}_MainActivity.txt" 2>/dev/null || echo "No MainActivity symbols found")
\`\`\`

### AppPlatform vtables
\`\`\`
$(grep "AppPlatform" "${output_dir}/${version}_vtables.txt" 2>/dev/null | head -20 || echo "No AppPlatform vtables found")
\`\`\`

### JNI Native Methods
\`\`\`
$(head -50 "${output_dir}/${version}_JNI_methods.txt" 2>/dev/null || echo "No JNI methods found")
\`\`\`

---

## Files Generated

- \`${version}_dynamic_symbols.txt\` - All dynamic (exported) symbols
- \`${version}_all_symbols.txt\` - All symbols (if debug info available)
- \`${version}_MainActivity.txt\` - MainActivity-related symbols
- \`${version}_AppPlatform.txt\` - AppPlatform-related symbols
- \`${version}_JNI_methods.txt\` - JNI native method symbols
- \`${version}_vtables.txt\` - Virtual table symbols
- \`${version}_system_property.txt\` - System property related symbols

---

## Usage for Comparison

To compare two versions:
\`\`\`bash
diff -u output/1.21.80_MainActivity.txt output/1.21.120_MainActivity.txt
diff -u output/1.21.80_JNI_methods.txt output/1.21.120_JNI_methods.txt
diff -u output/1.21.80_vtables.txt output/1.21.120_vtables.txt
\`\`\`

EOF

    echo -e "${GREEN}Report generated: $report_file${NC}"
}

main() {
    if [ $# -lt 1 ]; then
        usage
        exit 1
    fi

    local apk_file="$1"
    local arch="${2:-x86_64}"

    if [ ! -f "$apk_file" ]; then
        echo -e "${RED}APK file not found: $apk_file${NC}"
        exit 1
    fi

    # Extract version from filename
    local version=$(basename "$apk_file" .apk | sed 's/[^0-9.]//g' | head -c 20)
    if [ -z "$version" ]; then
        version="unknown"
    fi

    echo "============================================="
    echo "Minecraft APK Symbol Extraction Tool"
    echo "============================================="
    echo "APK: $apk_file"
    echo "Version: $version"
    echo "Architecture: $arch"
    echo "============================================="
    echo ""

    check_dependencies

    # Create output directory
    mkdir -p "$OUTPUT_DIR"

    # Extract APK
    local extract_dir="${OUTPUT_DIR}/extracted_${version}"
    extract_apk "$apk_file" "$extract_dir"

    # Find libraries
    local lib_dir=$(find_libraries "$extract_dir" "$arch")

    # Dump symbols from libminecraftpe.so
    local mcpe_lib="${lib_dir}/libminecraftpe.so"
    if [ -f "$mcpe_lib" ]; then
        dump_symbols "$mcpe_lib" "${OUTPUT_DIR}/${version}"
        filter_relevant_symbols "${OUTPUT_DIR}/${version}_dynamic_symbols.txt" "$OUTPUT_DIR" "$version"
    else
        echo -e "${RED}libminecraftpe.so not found!${NC}"
    fi

    # Dump symbols from libpairipcore.so if present
    local pairip_lib="${lib_dir}/libpairipcore.so"
    if [ -f "$pairip_lib" ]; then
        echo -e "${YELLOW}Found libpairipcore.so - extracting symbols...${NC}"
        dump_symbols "$pairip_lib" "${OUTPUT_DIR}/${version}_pairipcore"
    else
        echo -e "${YELLOW}libpairipcore.so not found (may not be present in this version)${NC}"
    fi

    # Generate report
    generate_report "$OUTPUT_DIR" "$version" "$arch"

    # Cleanup extracted files (keep symbols)
    echo -e "${YELLOW}Cleaning up extracted files...${NC}"
    rm -rf "$extract_dir"

    echo ""
    echo "============================================="
    echo -e "${GREEN}Symbol extraction complete!${NC}"
    echo "Output directory: $OUTPUT_DIR"
    echo "============================================="
}

main "$@"
