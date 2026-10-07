function(setup_picohash)

    FetchContent_Declare(
        picohash
        GIT_REPOSITORY https://github.com/kazuho/picohash.git
        GIT_TAG master
        SOURCE_DIR external/picohash
    )

    FetchContent_MakeAvailable(picohash)
    FetchContent_GetProperties(picohash)

    if(NOT TARGET picohash)
        add_library(picohash INTERFACE)
        target_include_directories(picohash INTERFACE "${picohash_SOURCE_DIR}")
    endif()
    if(TARGET picohash AND NOT TARGET PICOHASH::PICOHASH)
        add_library(PICOHASH::PICOHASH ALIAS picohash)
    endif()
endfunction()
