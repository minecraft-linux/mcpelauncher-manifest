/*
 * system_properties.cpp - Android System Properties Shim for Linux
 *
 * Complete implementation of Android system properties API required by
 * Minecraft Bedrock Edition 1.21.100+ (libpairipcore.so).
 *
 * This provides the necessary API for a legitimately purchased copy of
 * Minecraft to run on Linux via mcpelauncher.
 *
 * Copyright (c) 2025 minecraft-linux contributors
 * SPDX-License-Identifier: MIT
 */

#include "system_properties.h"
#include <cstring>
#include <map>
#include <string>
#include <mutex>

namespace shim {

// Thread-safe property storage
static std::mutex property_mutex;

// Fake Android system properties
// These values represent a legitimate Android device (Pixel-like)
// to satisfy pairipcore's device verification
static const std::map<std::string, std::string> system_properties = {
    // ==========================================================================
    // Build Information
    // ==========================================================================
    {"ro.build.id", "UP1A.231105.001"},
    {"ro.build.display.id", "UP1A.231105.001"},
    {"ro.build.version.incremental", "10817346"},
    {"ro.build.version.sdk", "34"},
    {"ro.build.version.preview_sdk", "0"},
    {"ro.build.version.preview_sdk_fingerprint", "REL"},
    {"ro.build.version.codename", "REL"},
    {"ro.build.version.all_codenames", "REL"},
    {"ro.build.version.known_codenames", "Base,Base11,Cupcake,Donut,Eclair,Eclair01,EclairMr1,Froyo,Gingerbread,GingerbreadMr1,Honeycomb,HoneycombMr1,HoneycombMr2,IceCreamSandwich,IceCreamSandwichMr1,JellyBean,JellyBeanMr1,JellyBeanMr2,Kitkat,KitkatWatch,Lollipop,LollipopMr1,M,N,NMr1,O,OMr1,P,Q,R,S,Sv2,Tiramisu,UpsideDownCake"},
    {"ro.build.version.release", "14"},
    {"ro.build.version.release_or_codename", "14"},
    {"ro.build.version.release_or_preview_display", "14"},
    {"ro.build.version.security_patch", "2024-01-05"},
    {"ro.build.version.base_os", ""},
    {"ro.build.version.min_supported_target_sdk", "28"},
    {"ro.build.date", "Thu Nov  9 00:34:22 UTC 2023"},
    {"ro.build.date.utc", "1699490062"},
    {"ro.build.type", "user"},
    {"ro.build.user", "android-build"},
    {"ro.build.host", "abfarm-release-rbe-2004-00041"},
    {"ro.build.tags", "release-keys"},
    {"ro.build.flavor", "oriole-user"},
    {"ro.build.description", "oriole-user 14 UP1A.231105.001 10817346 release-keys"},
    {"ro.build.fingerprint", "google/oriole/oriole:14/UP1A.231105.001/10817346:user/release-keys"},
    {"ro.build.characteristics", "default"},

    // ==========================================================================
    // Product Information (Pixel 6 style)
    // ==========================================================================
    {"ro.product.model", "Pixel 6"},
    {"ro.product.brand", "google"},
    {"ro.product.name", "oriole"},
    {"ro.product.device", "oriole"},
    {"ro.product.board", "oriole"},
    {"ro.product.manufacturer", "Google"},
    {"ro.product.locale", "en-US"},
    {"ro.product.first_api_level", "31"},
    {"ro.product.build.date", "Thu Nov  9 00:34:22 UTC 2023"},
    {"ro.product.build.date.utc", "1699490062"},
    {"ro.product.build.fingerprint", "google/oriole/oriole:14/UP1A.231105.001/10817346:user/release-keys"},
    {"ro.product.build.id", "UP1A.231105.001"},
    {"ro.product.build.tags", "release-keys"},
    {"ro.product.build.type", "user"},
    {"ro.product.build.version.incremental", "10817346"},
    {"ro.product.build.version.release", "14"},
    {"ro.product.build.version.release_or_codename", "14"},
    {"ro.product.build.version.sdk", "34"},

    // ==========================================================================
    // CPU/Architecture Information
    // ==========================================================================
    {"ro.product.cpu.abi", "arm64-v8a"},
    {"ro.product.cpu.abilist", "arm64-v8a,armeabi-v7a,armeabi"},
    {"ro.product.cpu.abilist32", "armeabi-v7a,armeabi"},
    {"ro.product.cpu.abilist64", "arm64-v8a"},

    // ==========================================================================
    // Hardware Information
    // ==========================================================================
    {"ro.hardware", "oriole"},
    {"ro.hardware.egl", "mali"},
    {"ro.hardware.vulkan", "mali"},
    {"ro.board.platform", "gs101"},
    {"ro.arch", "arm64"},
    {"ro.baseband", "g5123b-107837-231007-B-10689073"},
    {"gsm.version.baseband", "g5123b-107837-231007-B-10689073"},

    // ==========================================================================
    // Security/Bootloader State (appear as legitimate locked device)
    // ==========================================================================
    {"ro.boot.verifiedbootstate", "green"},
    {"ro.boot.veritymode", "enforcing"},
    {"ro.boot.flash.locked", "1"},
    {"ro.boot.vbmeta.device_state", "locked"},
    {"ro.boot.warranty_bit", "0"},
    {"ro.warranty_bit", "0"},
    {"ro.secure", "1"},
    {"ro.debuggable", "0"},
    {"ro.adb.secure", "1"},
    {"ro.allow.mock.location", "0"},
    {"ro.oem_unlock_supported", "1"},

    // ==========================================================================
    // SELinux
    // ==========================================================================
    {"ro.build.selinux", "0"},
    {"selinux.reload_policy", "1"},

    // ==========================================================================
    // System Properties
    // ==========================================================================
    {"ro.sys.sdcardfs", "false"},
    {"ro.url.legal", "http://www.google.com/intl/%s/mobile/android/basic/phone-legal.html"},
    {"ro.url.legal.android_privacy", "http://www.google.com/intl/%s/mobile/android/basic/privacy.html"},
    {"ro.com.google.clientidbase", "android-google"},
    {"ro.com.google.gmsversion", "14_202311"},

    // ==========================================================================
    // Locale/Language
    // ==========================================================================
    {"persist.sys.language", "en"},
    {"persist.sys.country", "US"},
    {"persist.sys.localevar", ""},
    {"persist.sys.timezone", "America/Los_Angeles"},
    {"ro.product.locale", "en-US"},

    // ==========================================================================
    // Network
    // ==========================================================================
    {"gsm.current.phone-type", "1"},
    {"gsm.network.type", "LTE"},
    {"gsm.nitz.time", "1699490062000"},
    {"gsm.operator.alpha", ""},
    {"gsm.operator.iso-country", "us"},
    {"gsm.operator.isroaming", "false"},
    {"gsm.operator.numeric", "310260"},
    {"gsm.sim.operator.alpha", ""},
    {"gsm.sim.operator.iso-country", "us"},
    {"gsm.sim.operator.numeric", "310260"},
    {"gsm.sim.state", "READY"},

    // ==========================================================================
    // Dalvik/ART VM
    // ==========================================================================
    {"dalvik.vm.appimageformat", "lz4"},
    {"dalvik.vm.dex2oat-Xms", "64m"},
    {"dalvik.vm.dex2oat-Xmx", "512m"},
    {"dalvik.vm.dex2oat-minidebuginfo", "true"},
    {"dalvik.vm.dex2oat-resolve-startup-strings", "true"},
    {"dalvik.vm.dex2oat-threads", "4"},
    {"dalvik.vm.dex2oat64.enabled", "true"},
    {"dalvik.vm.dexopt.secondary", "true"},
    {"dalvik.vm.heapgrowthlimit", "256m"},
    {"dalvik.vm.heapmaxfree", "8m"},
    {"dalvik.vm.heapminfree", "512k"},
    {"dalvik.vm.heapsize", "512m"},
    {"dalvik.vm.heapstartsize", "8m"},
    {"dalvik.vm.heaptargetutilization", "0.75"},
    {"dalvik.vm.image-dex2oat-Xms", "64m"},
    {"dalvik.vm.image-dex2oat-Xmx", "64m"},
    {"dalvik.vm.image-dex2oat-threads", "4"},
    {"dalvik.vm.isa.arm.features", "default"},
    {"dalvik.vm.isa.arm.variant", "cortex-a55"},
    {"dalvik.vm.isa.arm64.features", "default"},
    {"dalvik.vm.isa.arm64.variant", "cortex-a55"},
    {"dalvik.vm.minidebuginfo", "true"},
    {"dalvik.vm.stack-trace-dir", "/data/anr"},
    {"dalvik.vm.usejit", "true"},
    {"dalvik.vm.usejitprofiles", "true"},

    // ==========================================================================
    // Graphics/Display
    // ==========================================================================
    {"ro.opengles.version", "196610"},  // OpenGL ES 3.2
    {"ro.hardware.egl", "mali"},
    {"ro.hardware.vulkan", "mali"},
    {"debug.hwui.renderer", "skiavk"},
    {"graphics.gpu.profiler.support", "true"},

    // ==========================================================================
    // Feature Flags (Google Play compatible)
    // ==========================================================================
    {"ro.config.low_ram", "false"},
    {"ro.config.per_app_memcg", "true"},
    {"ro.config.avoid_gfx_accel", "false"},
    {"persist.sys.dalvik.vm.lib.2", "libart.so"},

    // ==========================================================================
    // Play Services/Store
    // ==========================================================================
    {"ro.com.google.gmsversion", "14_202311"},
    {"ro.setupwizard.mode", "OPTIONAL"},
    {"ro.opa.eligible_device", "true"},

    // ==========================================================================
    // Boot Properties
    // ==========================================================================
    {"ro.bootmode", "unknown"},
    {"ro.bootimage.build.date", "Thu Nov  9 00:34:22 UTC 2023"},
    {"ro.bootimage.build.date.utc", "1699490062"},
    {"ro.bootimage.build.fingerprint", "google/oriole/oriole:14/UP1A.231105.001/10817346:user/release-keys"},

    // ==========================================================================
    // Misc Properties
    // ==========================================================================
    {"ro.setupwizard.mode", "OPTIONAL"},
    {"ro.com.android.dateformat", "MM-dd-yyyy"},
    {"ro.com.android.dataroaming", "false"},
    {"ro.carrier", "unknown"},
    {"ro.config.ringtone", "Ring_Synth_04.ogg"},
    {"ro.config.notification_sound", "OnTheHunt.ogg"},
    {"ro.config.alarm_alert", "Alarm_Classic.ogg"},
    {"net.bt.name", "Android"},
    {"ro.kernel.qemu", "0"},
    {"ro.kernel.android.checkjni", "0"},
};

// Global serial number (incremented on changes)
static uint32_t global_serial = 1;

// ============================================================================
// __system_property_find - Find a property by name
// ============================================================================
const prop_info* __system_property_find(const char* name) {
    if (!name) return nullptr;

    std::lock_guard<std::mutex> lock(property_mutex);

    auto it = system_properties.find(name);
    if (it != system_properties.end()) {
        // Return a thread-local prop_info to avoid lifetime issues
        static thread_local prop_info pi;
        strncpy(pi.name, it->first.c_str(), PROP_NAME_MAX - 1);
        pi.name[PROP_NAME_MAX - 1] = '\0';
        strncpy(pi.value, it->second.c_str(), PROP_VALUE_MAX - 1);
        pi.value[PROP_VALUE_MAX - 1] = '\0';
        pi.serial = global_serial;
        return &pi;
    }
    return nullptr;
}

// ============================================================================
// __system_property_get - Get property value by name
// ============================================================================
int __system_property_get(const char* name, char* value) {
    if (!name || !value) {
        if (value) value[0] = '\0';
        return 0;
    }

    std::lock_guard<std::mutex> lock(property_mutex);

    auto it = system_properties.find(name);
    if (it != system_properties.end()) {
        strncpy(value, it->second.c_str(), PROP_VALUE_MAX - 1);
        value[PROP_VALUE_MAX - 1] = '\0';
        return static_cast<int>(strlen(value));
    }

    value[0] = '\0';
    return 0;
}

// ============================================================================
// __system_property_read - Read property (old API)
// ============================================================================
int __system_property_read(const prop_info* pi, char* name, char* value) {
    if (!pi) return 0;

    if (name) {
        strncpy(name, pi->name, PROP_NAME_MAX - 1);
        name[PROP_NAME_MAX - 1] = '\0';
    }
    if (value) {
        strncpy(value, pi->value, PROP_VALUE_MAX - 1);
        value[PROP_VALUE_MAX - 1] = '\0';
        return static_cast<int>(strlen(value));
    }
    return 0;
}

// ============================================================================
// __system_property_read_callback - Read property with callback (new API)
// ============================================================================
void __system_property_read_callback(
    const prop_info* pi,
    void (*callback)(void* cookie, const char* name, const char* value, uint32_t serial),
    void* cookie
) {
    if (!pi || !callback) return;
    callback(cookie, pi->name, pi->value, pi->serial);
}

// ============================================================================
// __system_property_foreach - Iterate all properties
// CRITICAL: This is what pairipcore uses for device verification
// ============================================================================
int __system_property_foreach(
    void (*propfn)(const prop_info* pi, void* cookie),
    void* cookie
) {
    if (!propfn) return -1;

    std::lock_guard<std::mutex> lock(property_mutex);

    // Use thread-local storage for the prop_info to avoid issues
    static thread_local prop_info pi;

    for (const auto& [key, val] : system_properties) {
        strncpy(pi.name, key.c_str(), PROP_NAME_MAX - 1);
        pi.name[PROP_NAME_MAX - 1] = '\0';
        strncpy(pi.value, val.c_str(), PROP_VALUE_MAX - 1);
        pi.value[PROP_VALUE_MAX - 1] = '\0';
        pi.serial = global_serial;

        propfn(&pi, cookie);
    }

    return 0;
}

// ============================================================================
// __system_property_serial - Get property serial number
// ============================================================================
uint32_t __system_property_serial(const prop_info* pi) {
    return pi ? pi->serial : 0;
}

// ============================================================================
// __system_property_wait - Wait for property change
// (stub - returns immediately since we don't have dynamic properties)
// ============================================================================
bool __system_property_wait(
    const prop_info* pi,
    uint32_t old_serial,
    uint32_t* new_serial_ptr,
    const struct timespec* relative_timeout
) {
    // We don't support dynamic property changes, so just return current serial
    if (new_serial_ptr) {
        *new_serial_ptr = pi ? pi->serial : global_serial;
    }
    return true;
}

// ============================================================================
// __system_property_area_serial - Get global serial
// ============================================================================
uint32_t __system_property_area_serial() {
    return global_serial;
}

// ============================================================================
// Register all shimmed symbols
// ============================================================================
void add_system_properties_shimmed_symbols(std::vector<shim::shimmed_symbol>& list) {
    list.insert(list.end(), {
        // Core functions
        {"__system_property_find", (void*) __system_property_find},
        {"__system_property_get", (void*) __system_property_get},
        {"__system_property_read", (void*) __system_property_read},
        {"__system_property_read_callback", (void*) __system_property_read_callback},

        // CRITICAL: foreach for pairipcore iteration
        {"__system_property_foreach", (void*) __system_property_foreach},

        // Serial/change detection
        {"__system_property_serial", (void*) __system_property_serial},
        {"__system_property_area_serial", (void*) __system_property_area_serial},
        {"__system_property_wait", (void*) __system_property_wait},
    });
}

} // namespace shim
