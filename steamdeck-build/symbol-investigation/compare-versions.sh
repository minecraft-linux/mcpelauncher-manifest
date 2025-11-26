#!/bin/bash
#
# Minecraft Version Symbol Comparison Tool
# Compares symbols between two Minecraft versions to identify changes
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/output"
COMPARE_DIR="${SCRIPT_DIR}/comparison"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

usage() {
    echo "Minecraft Version Symbol Comparison Tool"
    echo ""
    echo "Usage: $0 <old_version> <new_version>"
    echo ""
    echo "Arguments:"
    echo "  old_version   Version prefix for working version (e.g., 1.21.80)"
    echo "  new_version   Version prefix for broken version (e.g., 1.21.120)"
    echo ""
    echo "Prerequisites:"
    echo "  Run extract-symbols.sh on both APKs first"
    echo ""
    echo "Example:"
    echo "  ./extract-symbols.sh minecraft-1.21.80.apk"
    echo "  ./extract-symbols.sh minecraft-1.21.120.apk"
    echo "  ./compare-versions.sh 1.21.80 1.21.120"
}

compare_files() {
    local file1="$1"
    local file2="$2"
    local output_file="$3"
    local description="$4"

    echo -e "${YELLOW}Comparing: $description${NC}"

    if [ ! -f "$file1" ] || [ ! -f "$file2" ]; then
        echo -e "${RED}  Missing file(s) for comparison${NC}"
        return
    fi

    local added=$(comm -13 <(sort "$file1") <(sort "$file2") | wc -l)
    local removed=$(comm -23 <(sort "$file1") <(sort "$file2") | wc -l)
    local unchanged=$(comm -12 <(sort "$file1") <(sort "$file2") | wc -l)

    echo "  Added: $added | Removed: $removed | Unchanged: $unchanged"

    # Generate diff
    {
        echo "# $description"
        echo "# Old: $file1"
        echo "# New: $file2"
        echo "# Added: $added | Removed: $removed | Unchanged: $unchanged"
        echo ""
        echo "## REMOVED (in old, not in new):"
        comm -23 <(sort "$file1") <(sort "$file2")
        echo ""
        echo "## ADDED (in new, not in old):"
        comm -13 <(sort "$file1") <(sort "$file2")
    } > "$output_file"

    # Highlight critical changes
    if [ $removed -gt 0 ]; then
        echo -e "${RED}  CRITICAL: $removed symbols were REMOVED${NC}"
    fi
}

analyze_jni_changes() {
    local old_ver="$1"
    local new_ver="$2"
    local output_file="$3"

    echo -e "${CYAN}Analyzing JNI method changes...${NC}"

    local old_jni="${OUTPUT_DIR}/${old_ver}_JNI_methods.txt"
    local new_jni="${OUTPUT_DIR}/${new_ver}_JNI_methods.txt"

    if [ ! -f "$old_jni" ] || [ ! -f "$new_jni" ]; then
        echo -e "${RED}JNI files not found${NC}"
        return
    fi

    cat > "$output_file" << EOF
# JNI Method Changes Analysis
# Comparing: $old_ver -> $new_ver

## Summary

Old version JNI methods: $(wc -l < "$old_jni")
New version JNI methods: $(wc -l < "$new_jni")

## Critical: MainActivity Methods

### Removed MainActivity methods:
EOF

    comm -23 <(grep "MainActivity" "$old_jni" | sort) <(grep "MainActivity" "$new_jni" | sort) >> "$output_file"

    cat >> "$output_file" << EOF

### Added MainActivity methods:
EOF

    comm -13 <(grep "MainActivity" "$old_jni" | sort) <(grep "MainActivity" "$new_jni" | sort) >> "$output_file"

    cat >> "$output_file" << EOF

## Required Launcher Updates

Based on the changes above, the following updates may be needed in mcpelauncher-client:

1. **Removed methods**: These need to be removed from JNI registration
2. **Added methods**: These need to be implemented and registered
3. **Renamed methods**: Match old functionality to new names

See jni_support.cpp and main_activity.cpp in mcpelauncher-client.
EOF

    echo -e "${GREEN}JNI analysis complete: $output_file${NC}"
}

analyze_vtable_changes() {
    local old_ver="$1"
    local new_ver="$2"
    local output_file="$3"

    echo -e "${CYAN}Analyzing vtable changes...${NC}"

    local old_vt="${OUTPUT_DIR}/${old_ver}_vtables.txt"
    local new_vt="${OUTPUT_DIR}/${new_ver}_vtables.txt"

    if [ ! -f "$old_vt" ] || [ ! -f "$new_vt" ]; then
        echo -e "${RED}vtable files not found${NC}"
        return
    fi

    cat > "$output_file" << EOF
# VTable Symbol Changes Analysis
# Comparing: $old_ver -> $new_ver

## AppPlatform vtables

### Old version AppPlatform vtables:
$(grep "AppPlatform" "$old_vt" | sort || echo "None found")

### New version AppPlatform vtables:
$(grep "AppPlatform" "$new_vt" | sort || echo "None found")

## Changes

### Removed vtables:
$(comm -23 <(grep "AppPlatform" "$old_vt" | sort) <(grep "AppPlatform" "$new_vt" | sort) || echo "None")

### Added vtables:
$(comm -13 <(grep "AppPlatform" "$old_vt" | sort) <(grep "AppPlatform" "$new_vt" | sort) || echo "None")

## Required Launcher Updates

Update mcpelauncher-core/src/patch_utils.cpp to search for new vtable names.

Note: Since Minecraft 1.16.210+, the launcher uses hook-based patching instead of
vtable patching. The vtable errors may be non-fatal debug messages.
EOF

    echo -e "${GREEN}vtable analysis complete: $output_file${NC}"
}

analyze_pairipcore() {
    local old_ver="$1"
    local new_ver="$2"
    local output_file="$3"

    echo -e "${CYAN}Analyzing pairipcore changes...${NC}"

    local old_pp="${OUTPUT_DIR}/${old_ver}_pairipcore_dynamic_symbols.txt"
    local new_pp="${OUTPUT_DIR}/${new_ver}_pairipcore_dynamic_symbols.txt"

    cat > "$output_file" << EOF
# PairipCore Library Analysis
# Comparing: $old_ver -> $new_ver

## libpairipcore.so Presence

Old version ($old_ver): $([ -f "$old_pp" ] && echo "PRESENT" || echo "NOT PRESENT")
New version ($new_ver): $([ -f "$new_pp" ] && echo "PRESENT" || echo "NOT PRESENT")

EOF

    if [ -f "$new_pp" ]; then
        cat >> "$output_file" << EOF
## Required Symbols

libpairipcore.so requires these Android system functions from libc-shim:

- __system_property_foreach (iterate all properties)
- __system_property_read_callback (read property with callback)
- __system_property_find (find property by name)
- __system_property_get (get property value)

## Implementation Status

The libc-shim-patch in this repository provides implementations for all
required system property functions. Apply the patch to enable pairipcore support.

## Symbols Imported by libpairipcore.so
$(head -100 "$new_pp" 2>/dev/null || echo "Unable to read symbols")
EOF
    fi

    echo -e "${GREEN}pairipcore analysis complete: $output_file${NC}"
}

generate_comparison_report() {
    local old_ver="$1"
    local new_ver="$2"
    local report_file="${COMPARE_DIR}/COMPARISON_REPORT.md"

    echo -e "${YELLOW}Generating comparison report...${NC}"

    cat > "$report_file" << EOF
# Minecraft Symbol Comparison Report

**Old Version (Working):** $old_ver
**New Version (Broken):** $new_ver
**Generated:** $(date)

---

## Executive Summary

This report compares symbols between Minecraft versions to identify changes
that may be causing compatibility issues with mcpelauncher.

---

## Files Generated

| File | Description |
|------|-------------|
| \`diff_MainActivity.txt\` | MainActivity symbol changes |
| \`diff_AppPlatform.txt\` | AppPlatform symbol changes |
| \`diff_JNI_methods.txt\` | JNI native method changes |
| \`diff_vtables.txt\` | Virtual table changes |
| \`analysis_jni.md\` | Detailed JNI analysis |
| \`analysis_vtables.md\` | Detailed vtable analysis |
| \`analysis_pairipcore.md\` | PairipCore library analysis |

---

## Quick Reference

### Check for removed MainActivity methods:
\`\`\`bash
grep "^-" ${COMPARE_DIR}/diff_MainActivity.txt
\`\`\`

### Check for new JNI methods to implement:
\`\`\`bash
grep "^+" ${COMPARE_DIR}/diff_JNI_methods.txt
\`\`\`

### Check vtable name changes:
\`\`\`bash
cat ${COMPARE_DIR}/analysis_vtables.md
\`\`\`

---

## Next Steps

1. Review \`analysis_jni.md\` for required JNI updates
2. Review \`analysis_vtables.md\` for vtable search pattern updates
3. Apply libc-shim patch for system property support
4. Update mcpelauncher-client with new JNI methods
5. Test with Minecraft $new_ver

EOF

    echo -e "${GREEN}Comparison report generated: $report_file${NC}"
}

main() {
    if [ $# -lt 2 ]; then
        usage
        exit 1
    fi

    local old_ver="$1"
    local new_ver="$2"

    echo "============================================="
    echo "Minecraft Version Symbol Comparison"
    echo "============================================="
    echo "Old (working): $old_ver"
    echo "New (broken):  $new_ver"
    echo "============================================="
    echo ""

    # Check if symbol files exist
    if [ ! -f "${OUTPUT_DIR}/${old_ver}_dynamic_symbols.txt" ]; then
        echo -e "${RED}Symbol files for $old_ver not found!${NC}"
        echo "Run: ./extract-symbols.sh minecraft-${old_ver}.apk"
        exit 1
    fi

    if [ ! -f "${OUTPUT_DIR}/${new_ver}_dynamic_symbols.txt" ]; then
        echo -e "${RED}Symbol files for $new_ver not found!${NC}"
        echo "Run: ./extract-symbols.sh minecraft-${new_ver}.apk"
        exit 1
    fi

    # Create comparison directory
    mkdir -p "$COMPARE_DIR"

    # Compare different symbol categories
    compare_files \
        "${OUTPUT_DIR}/${old_ver}_MainActivity.txt" \
        "${OUTPUT_DIR}/${new_ver}_MainActivity.txt" \
        "${COMPARE_DIR}/diff_MainActivity.txt" \
        "MainActivity symbols"

    compare_files \
        "${OUTPUT_DIR}/${old_ver}_AppPlatform.txt" \
        "${OUTPUT_DIR}/${new_ver}_AppPlatform.txt" \
        "${COMPARE_DIR}/diff_AppPlatform.txt" \
        "AppPlatform symbols"

    compare_files \
        "${OUTPUT_DIR}/${old_ver}_JNI_methods.txt" \
        "${OUTPUT_DIR}/${new_ver}_JNI_methods.txt" \
        "${COMPARE_DIR}/diff_JNI_methods.txt" \
        "JNI methods"

    compare_files \
        "${OUTPUT_DIR}/${old_ver}_vtables.txt" \
        "${OUTPUT_DIR}/${new_ver}_vtables.txt" \
        "${COMPARE_DIR}/diff_vtables.txt" \
        "vtable symbols"

    # Generate detailed analyses
    analyze_jni_changes "$old_ver" "$new_ver" "${COMPARE_DIR}/analysis_jni.md"
    analyze_vtable_changes "$old_ver" "$new_ver" "${COMPARE_DIR}/analysis_vtables.md"
    analyze_pairipcore "$old_ver" "$new_ver" "${COMPARE_DIR}/analysis_pairipcore.md"

    # Generate overall report
    generate_comparison_report "$old_ver" "$new_ver"

    echo ""
    echo "============================================="
    echo -e "${GREEN}Comparison complete!${NC}"
    echo "Output directory: $COMPARE_DIR"
    echo "============================================="
}

main "$@"
