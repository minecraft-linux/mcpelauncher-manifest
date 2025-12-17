#include "no-fortify.h"
#include "system_properties.h"

#include <unistd.h>
#include <stdexcept>

void shim::add_system_properties_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.push_back({"__system_property_find", __system_property_find});
    list.push_back({"__system_property_get", __system_property_get});
    list.push_back({"__system_property_read_callback", __system_property_read_callback});
}

const prop_info* shim::__system_property_find(const char *name) {
    return 0;
}

int shim::__system_property_get(const char *name, char *value) {
    value[0] = 0;
    return 0;
}

void shim::__system_property_read_callback(const prop_info *pi, void (*callback)(void *cookie, const char *name, const char *value, uint32_t serial), void *cookie) {
    if(callback) {
        callback(cookie, "", "", 0);
    }
}
