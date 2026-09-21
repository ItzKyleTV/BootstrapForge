cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Tests
    LANGUAGES CXX
)

#
# Shared test configuration
#

function(
    bootstrapforge_configure_test
    target
)
    target_include_directories(
        ${target}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(
        ${target}
        PRIVATE
            BootstrapForge.Engine
            BootstrapForge.Domain
            BootstrapForge.Contracts
    )

    target_compile_features(
        ${target}
        PRIVATE
            cxx_std_23
    )

    if(MSVC)

        target_compile_options(
            ${target}
            PRIVATE
                /W4
                /permissive-
                /EHsc
        )

    else()

        target_compile_options(
            ${target}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )

    endif()

    set_target_properties(
        ${target}
        PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            FOLDER "BootstrapForge/Tests"
    )

    add_test(
        NAME ${target}
        COMMAND ${target}
    )
endfunction()

#
# Architecture Differ tests
#

add_executable(
    BootstrapForge.ArchitectureDifferTests
    src/ArchitectureDifferTests.cpp
)

bootstrapforge_configure_test(
    BootstrapForge.ArchitectureDifferTests
)

#
# Architecture Planner tests
#

add_executable(
    BootstrapForge.ArchitecturePlannerTests
    src/ArchitecturePlannerTests.cpp
)

bootstrapforge_configure_test(
    BootstrapForge.ArchitecturePlannerTests
)

#
# Execution Scheduler tests
#

add_executable(
    BootstrapForge.ExecutionSchedulerTests
    src/ExecutionSchedulerTests.cpp
)

bootstrapforge_configure_test(
    BootstrapForge.ExecutionSchedulerTests
)

#
# Execution Executor tests
#

add_executable(
    BootstrapForge.ExecutionExecutorTests
    src/ExecutionExecutorTests.cpp
)

bootstrapforge_configure_test(
    BootstrapForge.ExecutionExecutorTests
)