add_executable(
    BootstrapForge.ReconciliationIntegrationTests
    src/ReconciliationIntegrationTests.cpp
)

target_link_libraries(
    BootstrapForge.ReconciliationIntegrationTests
    PRIVATE
        BootstrapForge.Engine
        BootstrapForge.Application
        BootstrapForge.Infrastructure
        BootstrapForge.Domain
        BootstrapForge.Contracts
)

target_compile_features(
    BootstrapForge.ReconciliationIntegrationTests
    PRIVATE
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.ReconciliationIntegrationTests
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.ReconciliationIntegrationTests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.ReconciliationIntegrationTests
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "BootstrapForge/Tests"
)

add_test(
    NAME BootstrapForge.ReconciliationIntegrationTests
    COMMAND BootstrapForge.ReconciliationIntegrationTests
)

add_executable(
    BootstrapForge.ArchitectureParserTests
    src/ArchitectureParserTests.cpp
)

target_link_libraries(
    BootstrapForge.ArchitectureParserTests
    PRIVATE
        BootstrapForge.Architecture
)

target_compile_features(
    BootstrapForge.ArchitectureParserTests
    PRIVATE
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.ArchitectureParserTests
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.ArchitectureParserTests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.ArchitectureParserTests
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "BootstrapForge/Tests"
)

add_test(
    NAME BootstrapForge.ArchitectureParserTests
    COMMAND BootstrapForge.ArchitectureParserTests
)

add_executable(
    BootstrapForge.GenerationTests
    src/GenerationTests.cpp
)

target_include_directories(
    BootstrapForge.GenerationTests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    BootstrapForge.GenerationTests
    PRIVATE
        BootstrapForge.Architecture
        BootstrapForge.Infrastructure
)

target_compile_features(
    BootstrapForge.GenerationTests
    PRIVATE
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.GenerationTests
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.GenerationTests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.GenerationTests
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "BootstrapForge/Tests"
)

add_test(
    NAME BootstrapForge.GenerationTests
    COMMAND BootstrapForge.GenerationTests
)
