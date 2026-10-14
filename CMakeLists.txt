cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge
    VERSION 0.1.0
    DESCRIPTION "Deterministic architecture reconciliation engine."
    LANGUAGES C CXX
)

#
# Compiler
#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#
# Output
#

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Third Party
#

include(cmake/Dependencies.cmake)

#
# Core Libraries
#

add_subdirectory(Domain)
add_subdirectory(Contracts)
add_subdirectory(Architecture)

#
# Application Layer
#

add_subdirectory(Application)
add_subdirectory(Infrastructure)

#
# Engine Layer
#

add_subdirectory(Engine)

#
# AI Capabilities
#

add_subdirectory(Capabilities)

#
# Runtime
#

add_subdirectory(Runtime)

#
# Tests
#

enable_testing()
add_subdirectory(Tests)

#
# Client
#

add_subdirectory(Client)
