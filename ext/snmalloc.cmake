include(FetchContent)

set(SNMALLOC_HEADER_ONLY_LIBRARY ON)
set(SNMALLOC_USE_CXX17 ON)

FetchContent_Declare(
        snmalloc_ext
        URL "https://github.com/microsoft/snmalloc/archive/refs/tags/0.6.1.zip"
)

FetchContent_GetProperties(snmalloc_ext)
if(NOT snmalloc_ext_POPULATED)
  FetchContent_Populate(snmalloc_ext)
  add_subdirectory(${snmalloc_ext_SOURCE_DIR} ${snmalloc_ext_BINARY_DIR})
endif()