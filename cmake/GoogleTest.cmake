function(setup_gtest)
    # Force static linking to avoid DLL issues
    set(gtest_force_shared_crt ON CACHE BOOL "Use shared CRT" FORCE)
    set(gtest_build_tests OFF CACHE BOOL "Disable GTest tests" FORCE)
    set(gtest_build_samples OFF CACHE BOOL "Disable GTest samples" FORCE)
    set(gmock_build_tests OFF CACHE BOOL "Disable GMock tests" FORCE)
    set(gtest_disable_pthreads ON CACHE BOOL "Disable pthreads" FORCE)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        SOURCE_DIR external/googletest
    )
    
    FetchContent_MakeAvailable(googletest)
endfunction()
