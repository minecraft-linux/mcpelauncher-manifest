#pragma once

#include <libc_shim.h>

typedef struct prop_info prop_info;

namespace shim {
    const prop_info *__system_property_find(const char *name);
    int __system_property_get(const char *name, char *value);
    void __system_property_read_callback(const prop_info* pi, void (* callback)(void* cookie, const char* name, const char* value, uint32_t serial), void* cookie);

    void add_system_properties_shimmed_symbols(std::vector<shimmed_symbol> &list);

}
