function(setup_wuffs)
     FetchContent_Declare(
        wuffs
        GIT_REPOSITORY https://github.com/google/wuffs.git
        GIT_TAG v0.3.5
        SOURCE_DIR external/wuffs
    )
  
    FetchContent_MakeAvailable(wuffs)
    set(WUFFS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/WuffsImplementation.cpp
    )
    add_library(wuffs STATIC ${WUFFS_SOURCES})
    target_compile_definitions(wuffs PRIVATE
        WUFFS_CONFIG__MODULES
        WUFFS_CONFIG__MODULE__ADLER32
        WUFFS_CONFIG__MODULE__CRC32
        WUFFS_CONFIG__MODULE__DEFLATE
        WUFFS_CONFIG__MODULE__ZLIB
        WUFFS_CONFIG__MODULE__AUX__IMAGE
        WUFFS_CONFIG__MODULE__AUX__BASE
        WUFFS_CONFIG__MODULE__BASE
        WUFFS_CONFIG__MODULE__PNG
        WUFFS_CONFIG__MODULE__JPEG)
    target_link_libraries(wuffs )
    target_include_directories(wuffs PUBLIC ${wuffs_SOURCE_DIR}/release/c)

endfunction()
