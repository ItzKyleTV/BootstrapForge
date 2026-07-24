cmake_minimum_required(VERSION 3.20)

project(BootstrapForge.Domain LANGUAGES CXX)

add_library(BootstrapForge.Domain)

target_compile_features(
    BootstrapForge.Domain
    PUBLIC
        cxx_std_23
)

target_include_directories(
    BootstrapForge.Domain
    PUBLIC
        include
)

target_sources(
    BootstrapForge.Domain
    PRIVATE

        #
        # Types
        #
        src/Types/Hash.cpp
        src/Types/Identifier.cpp
        src/Types/Name.cpp
        src/Types/Path.cpp

        #
        # Operations
        #
        src/Operations/ExecutionPlan.cpp
        src/Operations/ExecutionResult.cpp
        src/Operations/ExecutionResultEntry.cpp

        #
        # Models
        #
        src/Models/ArchitectureModel.cpp
        src/Models/Relationship.cpp
        src/Models/Resource.cpp
)

if(MSVC)

    target_compile_options(
        BootstrapForge.Domain
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.Domain
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.Domain
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)