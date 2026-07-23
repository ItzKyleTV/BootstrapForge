cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Runtime LANGUAGES CXX)

add_executable(BootstrapForge.Runtime
    src/main.cpp
)

target_link_libraries(BootstrapForge.Runtime
    PRIVATE
        BootstrapForge.Domain
        BootstrapForge.Engine
)

target_compile_features(BootstrapForge.Runtime
    PRIVATE
        cxx_std_23
)

if(MSVC)

    target_compile_options(BootstrapForge.Runtime PRIVATE
        /W4
        /permissive-
        /EHsc
    )

else()

    target_compile_options(BootstrapForge.Runtime PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()