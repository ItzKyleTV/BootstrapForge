cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Application LANGUAGES CXX)

add_library(BootstrapForge.Application STATIC)

target_sources(BootstrapForge.Application
    PRIVATE
        src/Identity/RelationshipFactory.cpp
        src/Identity/ResourceFactory.cpp


        "src/Mapping/ManifestMapper.cpp"

        src/Generation/GenerationPlanner.cpp

        src/Validation/ManifestValidator.cpp
)

target_include_directories(BootstrapForge.Application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BootstrapForge.Application
    PUBLIC
        BootstrapForge.Domain
        BootstrapForge.Contracts
        BootstrapForge.Architecture
)

target_compile_features(BootstrapForge.Application
    PUBLIC
        cxx_std_23
)

if(MSVC)
    target_compile_options(BootstrapForge.Application PRIVATE
        /W4
        /permissive-
        /EHsc
    )
else()
    target_compile_options(BootstrapForge.Application PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

set_target_properties(BootstrapForge.Application PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
