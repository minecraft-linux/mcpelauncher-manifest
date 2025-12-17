#pragma once

#include <libc_shim.h>

namespace shim {

    namespace bionic {

        int to_host_sysconf_id(int id);

    }

    long sysconf(int name);

    void add_sysconf_shimmed_symbols(std::vector<shimmed_symbol> &list);

}