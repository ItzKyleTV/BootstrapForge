cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Capabilities
    LANGUAGES CXX
)

add_library(BootstrapForge.Capabilities INTERFACE)

target_link_libraries(
    BootstrapForge.Capabilities
    INTERFACE
        BootstrapForge.Application
)

target_compile_features(
    BootstrapForge.Capabilities
    INTERFACE
        cxx_std_23
)

set_target_properties(
    BootstrapForge.Capabilities
    PROPERTIES
        FOLDER "BootstrapForge"
)