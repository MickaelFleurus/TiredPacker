function(setup_rectpack2d)
     FetchContent_Declare(
        rectpack2d
        GIT_REPOSITORY https://github.com/TeamHypersomnia/rectpack2D.git
        GIT_TAG master
        SOURCE_DIR external/rectpack2d
    )
    FetchContent_MakeAvailable(rectpack2d)
endfunction()