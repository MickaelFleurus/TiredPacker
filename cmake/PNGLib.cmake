function(setup_pnglib)
     FetchContent_Declare(
        libpng
        GIT_REPOSITORY https://github.com/glennrp/libpng.git
        GIT_TAG v1.6.50
        SOURCE_DIR external/libpng
    )
    set(PNG_SHARED OFF CACHE BOOL "" FORCE)
    set(PNG_STATIC ON CACHE BOOL "" FORCE)
    set(PNG_TESTS OFF CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_LIBRARIES ON CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libpng)
    if(TARGET png_static AND NOT TARGET PNG::PNG)
        add_library(PNG::PNG ALIAS png_static)
    endif()
endfunction()