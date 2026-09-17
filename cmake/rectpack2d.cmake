function(setup_rectpack2d)
    FetchContent_Declare(
        rectpack2d
        GIT_REPOSITORY https://github.com/TeamHypersomnia/rectpack2D.git
        GIT_TAG master
        SOURCE_DIR external/rectpack2d
    )
    FetchContent_MakeAvailable(rectpack2d)

    if(TARGET rectpack2D AND NOT TARGET rectpack2D::rectpack2D)
        add_library(rectpack2D::rectpack2D ALIAS rectpack2D)
    endif()

    if(TARGET rectpack2D AND NOT TARGET rectpack2d::rectpack2d)
        add_library(rectpack2d::rectpack2d ALIAS rectpack2D)
    endif()
endfunction()
