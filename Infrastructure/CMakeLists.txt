cmake_minimum_required(VERSION 3.28)

project(BootstrapForge.Infrastructure LANGUAGES CXX)

add_library(BootstrapForge.Infrastructure STATIC)

target_sources(
    BootstrapForge.Infrastructure
    PRIVATE

        #
        # Cryptography
        #

        src/Cryptography/Hashing/Blake3Hasher.cpp

        #
        # Deserialization
        #

        "src/Execution/JsonManifestDeserializer.cpp"

        #
        # Filesystem
        #

        src/Filesystem/ManifestFileReader.cpp
        src/Filesystem/PhysicalFilesystemScanner.cpp

        #
        # Loading
        #

        src/Loading/JsonDesiredArchitectureLoader.cpp

        #
        # Execution
        #

        src/Execution/NullExecutionBackend.cpp
        src/Execution/PhysicalExecutionBackend.cpp

        src/Generation/GenerationExecutor.cpp
)

target_include_directories(
    BootstrapForge.Infrastructure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    BootstrapForge.Infrastructure
    PUBLIC
        BootstrapForge.Contracts
        BootstrapForge.Application
        blake3
        nlohmann_json::nlohmann_json
)

target_compile_features(
    BootstrapForge.Infrastructure
    PUBLIC
        cxx_std_23
)

if(MSVC)

    target_compile_options(
        BootstrapForge.Infrastructure
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )

else()

    target_compile_options(
        BootstrapForge.Infrastructure
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

set_target_properties(
    BootstrapForge.Infrastructure
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
