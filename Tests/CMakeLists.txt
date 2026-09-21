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