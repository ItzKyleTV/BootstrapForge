
cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Client
    VERSION 0.1.0
    DESCRIPTION "BootstrapForge desktop client."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(
    Qt6 6.11
    REQUIRED
    COMPONENTS Core Gui Widgets
)

qt_standard_project_setup()

if(WIN32)
    get_filename_component(
        BOOTSTRAPFORGE_QT_CMAKE_DIRECTORY
        "${Qt6_DIR}"
        ABSOLUTE
    )

    get_filename_component(
        BOOTSTRAPFORGE_QT_CMAKE_PARENT
        "${BOOTSTRAPFORGE_QT_CMAKE_DIRECTORY}"
        DIRECTORY
    )

    get_filename_component(
        BOOTSTRAPFORGE_QT_LIB_DIRECTORY
        "${BOOTSTRAPFORGE_QT_CMAKE_PARENT}"
        DIRECTORY
    )

    get_filename_component(
        BOOTSTRAPFORGE_QT_PREFIX
        "${BOOTSTRAPFORGE_QT_LIB_DIRECTORY}"
        DIRECTORY
    )

    find_program(
        BOOTSTRAPFORGE_WINDEPLOYQT
        NAMES windeployqt.exe
        HINTS "${BOOTSTRAPFORGE_QT_PREFIX}/bin"
        REQUIRED
    )
endif()

add_executable(
    BootstrapForge.Client
    src/main.cpp

    src/Editor/EditorDocument.cpp
    src/Editor/EditorManager.cpp
    src/Editor/CppSyntaxHighlighter.cpp

    src/Project/ProjectTree.cpp
    src/UI/MainWindow.cpp

    include/BootstrapForge/Client/Editor/EditorDocument.hpp
    include/BootstrapForge/Client/Editor/EditorManager.hpp
    include/BootstrapForge/Client/Editor/CppSyntaxHighlighter.hpp
    include/BootstrapForge/Client/Project/ProjectTree.hpp
    include/BootstrapForge/Client/UI/MainWindow.hpp
)

target_include_directories(
    BootstrapForge.Client
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(
    BootstrapForge.Client
    PRIVATE
        BootstrapForge.Domain
        BootstrapForge.Contracts
        BootstrapForge.Application
        BootstrapForge.Engine
        BootstrapForge.Infrastructure
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_features(
    BootstrapForge.Client
    PRIVATE
        cxx_std_23
)

if(MSVC)
    target_compile_options(
        BootstrapForge.Client
        PRIVATE
            /W4
            /permissive-
            /EHsc
    )
else()
    target_compile_options(
        BootstrapForge.Client
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

set_target_properties(
    BootstrapForge.Client
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        FOLDER "BootstrapForge/Client"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(
        TARGET BootstrapForge.Client
        POST_BUILD
        COMMAND
            "${BOOTSTRAPFORGE_WINDEPLOYQT}"
            "--$<LOWER_CASE:$<CONFIG>>"
            --no-translations
            "$<TARGET_FILE:BootstrapForge.Client>"
        COMMENT "Deploying Qt runtime for BootstrapForge.Client"
        VERBATIM
    )
endif()
