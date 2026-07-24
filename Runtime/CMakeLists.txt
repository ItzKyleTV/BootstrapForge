cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Runtime LANGUAGES CXX)

add_executable(
    BootstrapForge.Runtime
    src/main.cpp
)

target_link_libraries(
    BootstrapForge.Runtime
    PRIVATE
        BootstrapForge.Domain
        BootstrapForge.Engine
        BootstrapForge.Application
        BootstrapForge.Infrastructure
)

target_compile_features(
    BootstrapForge.Runtime
    PRIVATE
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.Runtime
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.Runtime
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.Runtime
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)