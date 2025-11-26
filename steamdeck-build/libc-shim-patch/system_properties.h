/*
 * system_properties.h - Android System Properties Shim for Linux
 *
 * This implementation provides the Android system properties API required
 * by Minecraft Bedrock Edition 1.21.100+ which uses Google's PairIP Core
 * protection library (libpairipcore.so).
 *
 * This is for legitimate use - running a properly purchased copy of
 * Minecraft on Linux via mcpelauncher.
 *
 * Copyright (c) 2025 minecraft-linux contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <libc_shim.h>
#include <vector>
#include <functional>
#include <cstdint>

namespace shim {

// Android property constants
constexpr int PROP_NAME_MAX = 32;
constexpr int PROP_VALUE_MAX = 92;

// Opaque property info structure
struct prop_info {
    char name[PROP_NAME_MAX];
    char value[PROP_VALUE_MAX];
    uint32_t serial;
};

// Core property functions
const prop_info* __system_property_find(const char* name);
int __system_property_get(const char* name, char* value);

// Old API (pre-API 26)
int __system_property_read(const prop_info* pi, char* name, char* value);

// New API (API 26+)
void __system_property_read_callback(
    const prop_info* pi,
    void (*callback)(void* cookie, const char* name, const char* value, uint32_t serial),
    void* cookie
);

// Iteration - CRITICAL for pairipcore
int __system_property_foreach(
    void (*propfn)(const prop_info* pi, void* cookie),
    void* cookie
);

// Serial number for change detection
uint32_t __system_property_serial(const prop_info* pi);

// Wait for changes (stub - returns immediately)
bool __system_property_wait(
    const prop_info* pi,
    uint32_t old_serial,
    uint32_t* new_serial_ptr,
    const struct timespec* relative_timeout
);

// Global serial (stub)
uint32_t __system_property_area_serial();

// Symbol registration
void add_system_properties_shimmed_symbols(std::vector<shim::shimmed_symbol>& list);

} // namespace shim
