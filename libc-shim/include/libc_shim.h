#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace shim {

    struct shimmed_symbol {
        const char *name;
        void *value;

        shimmed_symbol(const char *name, void *value)
            : name(name), value(value) {}

        template <typename Ret, typename ...Args>
        shimmed_symbol(const char *name, Ret (*ptr)(Args...))
            : name(name), value((void*) ptr) {}

        template <typename Ret, typename ...Args>
        shimmed_symbol(const char *name, Ret (*ptr)(Args..., ...))
                : name(name), value((void*) ptr) {}
    };

    std::vector<shimmed_symbol> get_shimmed_symbols();

    // Rewrite filesystem access
    extern std::vector<std::pair<std::string, std::string>> rewrite_filesystem_access;

    [[noreturn]] void handle_runtime_error(const char* fmt, ...);
}