function(setup_fpng)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0
       AND NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
        set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING
            "Minimum policy version for legacy dependencies")
    endif()

    FetchContent_Declare(
        fpng
        GIT_REPOSITORY https://github.com/richgel999/fpng.git
        GIT_TAG main
        SOURCE_DIR external/fpng
    )

    FetchContent_MakeAvailable(fpng)
    FetchContent_GetProperties(fpng)

    if(NOT TARGET fpng)
        add_library(fpng STATIC
            "${fpng_SOURCE_DIR}/src/fpng.cpp"
            "${fpng_SOURCE_DIR}/src/lodepng.cpp"
            "${fpng_SOURCE_DIR}/src/pvpngreader.cpp")
        target_include_directories(fpng PUBLIC "${fpng_SOURCE_DIR}/src")
    endif()
    if(TARGET fpng AND NOT TARGET FPNG::FPNG)
        add_library(FPNG::FPNG ALIAS fpng)
    endif()
endfunction()
