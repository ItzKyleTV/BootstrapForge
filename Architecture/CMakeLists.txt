cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Architecture
    VERSION 0.1.0
    DESCRIPTION "BootstrapForge Architecture Compiler"
    LANGUAGES CXX
)

#
# Library
#

add_library(
    BootstrapForge.Architecture
    STATIC
)

add_library(
    BootstrapForge::Architecture
    ALIAS
    BootstrapForge.Architecture
)

#
# Sources
#

set(
    ARCHITECTURE_SOURCES

    #
    # Grammar
    #

    src/Grammar/Grammar.cpp
    src/Grammar/KeywordTable.cpp
    src/Grammar/OperatorTable.cpp

    #
    # Parsing
    #

    src/Parsing/SourceReader.cpp
    src/Parsing/Lexer.cpp
    src/Parsing/TokenStream.cpp

    #
    # AST
    #

    src/AST/Parser.cpp

    #
    # IR
    #

    src/IR/IRBuilder.cpp

    #
    # Diagnostics
    #

    src/Diagnostics/DiagnosticBag.cpp
    src/Diagnostics/DiagnosticEngine.cpp
    src/Diagnostics/DiagnosticFormatter.cpp
    src/Diagnostics/DiagnosticRenderer.cpp

    #
    # Expansion
    #

    #
    # Serialization
    #

    #
    # Templates
    #

    #
    # Validation
    #
)

#
# Sources
#

target_sources(
    BootstrapForge.Architecture
    PRIVATE
        ${ARCHITECTURE_SOURCES}
)

#
# Include Directories
#

target_include_directories(
    BootstrapForge.Architecture
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#
# Dependencies
#

target_link_libraries(
    BootstrapForge.Architecture
    PUBLIC
        BootstrapForge.Domain
        BootstrapForge.Contracts
)

#
# Compiler Features
#

target_compile_features(
    BootstrapForge.Architecture
    PUBLIC
        cxx_std_23
)

#
# Compiler Options
#

if(MSVC)

    target_compile_options(
        BootstrapForge.Architecture
        PRIVATE
            /W4
            /permissive-
            /EHsc
            /Zc:__cplusplus
    )

else()

    target_compile_options(
        BootstrapForge.Architecture
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

#
# IDE Source Groups
#

source_group(
    TREE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX
        "Source Files"
    FILES
        ${ARCHITECTURE_SOURCES}
)

#
# Target Properties
#

set_target_properties(
    BootstrapForge.Architecture
    PROPERTIES

        VERSION 0.1.0

        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF

        FOLDER "BootstrapForge"
)