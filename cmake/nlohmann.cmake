function(setup_nlohmann_json)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        SOURCE_DIR external/nlohmann_json
    )
    FetchContent_MakeAvailable(nlohmann_json)

endfunction()