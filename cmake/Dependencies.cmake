include(FetchContent)

#
# Blake3
#

FetchContent_Declare(
    blake3
    GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
    GIT_TAG 1.8.2
    SOURCE_SUBDIR c
)

#
# nlohmann-json
#

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(
    blake3
    nlohmann_json
)