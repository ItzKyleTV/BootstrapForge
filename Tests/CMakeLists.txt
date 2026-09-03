cmake_minimum_required(VERSION 3.28)

project(
    BootstrapForge.Tests
    LANGUAGES CXX
)

#
# Placeholder until the testing framework is implemented.
#

add_custom_target(
    BootstrapForge.Tests
)

set_target_properties(
    BootstrapForge.Tests
    PROPERTIES
        FOLDER "BootstrapForge/Tests"
)