/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <cstring>

namespace shim {

    namespace bionic {

        struct mallinfo {
        /** Total number of non-mmapped bytes currently allocated from OS. */
        size_t arena;
        /** Number of free chunks. */
        size_t ordblks;
        /** (Unused.) */
        size_t smblks;
        /** (Unused.) */
        size_t hblks;
        /** Total number of bytes in mmapped regions. */
        size_t hblkhd;
        /** Maximum total allocated space; greater than total if trimming has occurred. */
        size_t usmblks;
        /** (Unused.) */
        size_t fsmblks;
        /** Total allocated space (normal or mmapped.) */
        size_t uordblks;
        /** Total free space. */
        size_t fordblks;
        /** Upper bound on number of bytes releasable by a trim operation. */
        size_t keepcost;
        };
    }
}