include(ExternalProject)

if (NOT APPLE)
    find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
endif()

if (NOT CURL_EXT_EXTRA_OPTIONS)
    set(CURL_EXT_EXTRA_OPTIONS )
endif()

if (DEFINED OPENSSL_ROOT_DIR)
    list(APPEND CURL_EXT_EXTRA_OPTIONS "-DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}")
endif()

ExternalProject_Add(
        curl_ext
        URL "http://curl.haxx.se/download/curl-7.72.0.tar.gz"
        INSTALL_DIR ${CMAKE_BINARY_DIR}/ext/curl
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/ext/curl" "-DBUILD_CURL_EXE=OFF" "-DBUILD_SHARED_LIBS=OFF" "-DCURL_STATICLIB=ON" "-DCURL_DISABLE_LDAP=ON" "-DCMAKE_USE_LIBSSH2=OFF" "-DCMAKE_USE_OPENLDAP=OFF" "-DCMAKE_C_FLAGS=-m32" "-DCMAKE_LINK_FLAGS=-m32" "-DCMAKE_LIBRARY_ARCHITECTURE=${CMAKE_LIBRARY_ARCHITECTURE}" ${CURL_EXT_EXTRA_OPTIONS}
)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/curl/include/)
add_library(curl STATIC IMPORTED)
add_dependencies(curl curl_ext)
set_property(TARGET curl PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/ext/curl/lib/libcurl.a)
set_property(TARGET curl PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/ext/curl/include/)
if (NOT APPLE)
    set_property(TARGET curl PROPERTY INTERFACE_LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
endif()

set(CURL_FOUND TRUE)
set(CURL_LIBRARIES curl)
set(CURL_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/ext/curl/include/)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/ext/cmake_find_stubs)
file(WRITE ${CMAKE_BINARY_DIR}/ext/cmake_find_stubs/FindCURL.cmake "")
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/ext/cmake_find_stubs" ${CMAKE_MODULE_PATH})
