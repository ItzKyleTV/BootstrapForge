cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Infrastructure LANGUAGES C CXX)

add_library(BootstrapForge.Infrastructure STATIC)

target_sources(BootstrapForge.Infrastructure
    PRIVATE

        src/Cryptography/Hashing/Blake3Hasher.cpp

        Vendor/blake3/c/blake3.c
        Vendor/blake3/c/blake3_dispatch.c
        Vendor/blake3/c/blake3_portable.c
)

target_include_directories(BootstrapForge.Infrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/blake3/c
)

target_link_libraries(BootstrapForge.Infrastructure
    PUBLIC
        BootstrapForge.Contracts
)

target_compile_features(BootstrapForge.Infrastructure
    PUBLIC
        cxx_std_23
)

if(MSVC)

    target_compile_options(BootstrapForge.Infrastructure PRIVATE
        /W4
        /permissive-
        /EHsc
    )

else()

    target_compile_options(BootstrapForge.Infrastructure PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()

set_target_properties(BootstrapForge.Infrastructure PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON

    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)