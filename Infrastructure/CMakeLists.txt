cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Infrastructure LANGUAGES C CXX ASM)

#
# Vendor
#

add_subdirectory(Vendor/blake3/c)

#
# Infrastructure
#

add_library(BootstrapForge.Infrastructure STATIC)

target_sources(BootstrapForge.Infrastructure
    PRIVATE
        src/Cryptography/Hashing/Blake3Hasher.cpp
 "src/Filesystem/PhysicalFileSystem.cpp" "src/Execution/NullExecutionBackend.cpp")

target_include_directories(BootstrapForge.Infrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BootstrapForge.Infrastructure
    PUBLIC
        BootstrapForge.Contracts
        blake3
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


    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)