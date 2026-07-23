cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge
    VERSION 0.1.0
    DESCRIPTION "Deterministic architecture reconciliation engine."
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(Domain)
add_subdirectory(Contracts)
add_subdirectory(Infrastructure)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/CMakeLists.txt")
    add_subdirectory(Engine)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Application/CMakeLists.txt")
    add_subdirectory(Application)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Capabilities/CMakeLists.txt")
    add_subdirectory(Capabilities)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Runtime/CMakeLists.txt")
    add_subdirectory(Runtime)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(Tests)
endif()
