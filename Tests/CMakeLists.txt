cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Tests
    LANGUAGES CXX
)

#
# Test executable
#

add_executable(
    BootstrapForge.Tests
    src/ArchitectureDifferTests.cpp
)

#
# Dependencies
#

target_link_libraries(
    BootstrapForge.Tests
    PRIVATE
        BootstrapForge.Engine
        BootstrapForge.Domain
)

#
# Compiler features
#

target_compile_features(
    BootstrapForge.Tests
    PRIVATE
        cxx_std_23
)

#
# Compiler options
#

if(MSVC)

    target_compile_options(
        BootstrapForge.Tests
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.Tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

#
# Target properties
#

set_target_properties(
    BootstrapForge.Tests
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "BootstrapForge/Tests"
)

#
# CTest
#

add_test(
    NAME BootstrapForge.Tests
    COMMAND BootstrapForge.Tests
)