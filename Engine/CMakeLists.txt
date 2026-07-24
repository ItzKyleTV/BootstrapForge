cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Engine LANGUAGES CXX)

add_library(BootstrapForge.Engine STATIC)

target_sources(
    BootstrapForge.Engine
    PRIVATE

        #
        # Differ
        #
        src/Differ/ArchitectureDiffer.cpp

        #
        # Planner
        #
        src/Planner/ArchitecturePlanner.cpp

        #
        # Scheduler
        #
        src/Scheduler/ExecutionScheduler.cpp

        #
        # Formatting
        #
        src/Formatting/ExecutionExecutor.cpp
)

target_include_directories(
    BootstrapForge.Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    BootstrapForge.Engine
    PUBLIC
        BootstrapForge.Domain
)

target_compile_features(
    BootstrapForge.Engine
    PUBLIC
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.Engine
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.Engine
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.Engine
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)