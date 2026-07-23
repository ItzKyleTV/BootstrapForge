cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Domain LANGUAGES CXX)

add_library(BootstrapForge.Domain STATIC)

target_sources(BootstrapForge.Domain
    PRIVATE
        src/Types/Hash.cpp
        src/Types/Identifier.cpp
        src/Types/Name.cpp
        src/Types/Path.cpp
)

target_include_directories(BootstrapForge.Domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(BootstrapForge.Domain
    PUBLIC
        cxx_std_23
)

if(MSVC)

    target_compile_options(BootstrapForge.Domain PRIVATE
        /W4
        /permissive-
        /EHsc
    )

else()

    target_compile_options(BootstrapForge.Domain PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()

set_target_properties(BootstrapForge.Domain PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)