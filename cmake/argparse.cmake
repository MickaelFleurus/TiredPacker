function(setup_argparse)
    FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG master
        SOURCE_DIR external/argparse
    )
    FetchContent_MakeAvailable(argparse)

endfunction()
