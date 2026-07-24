cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Contracts LANGUAGES CXX)

add_library(BootstrapForge.Contracts INTERFACE)

target_sources(
    BootstrapForge.Contracts
    INTERFACE

        #
        # Services
        #
        include/BootstrapForge/Contracts/Services/Cryptography/IHasher.hpp

        #
        # Execution
        #
        include/BootstrapForge/Contracts/Execution/IExecutionBackend.hpp
)

target_include_directories(
    BootstrapForge.Contracts
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    BootstrapForge.Contracts
    INTERFACE
        BootstrapForge.Domain
)

target_compile_features(
    BootstrapForge.Contracts
    INTERFACE
        cxx_std_23
)