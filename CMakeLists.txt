cmake_minimum_required(VERSION 4.4)

# Awful but necessary because of ninja apparently.
if(CMAKE_HOST_APPLE AND NOT CMAKE_OSX_SYSROOT)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        RESULT_VARIABLE MACOS_SDK_RESULT
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(MACOS_SDK_RESULT EQUAL 0 AND MACOS_SDK_PATH)
        set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}" CACHE PATH "macOS SDK used for compilation" FORCE)
    endif()
endif()

project(TiredPacker VERSION 1.0 LANGUAGES C CXX)
include(FetchContent)

set(ATLAS_METADATA_VERSION 1)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
if(WIN32 AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG_BUILD>)

include(nlohmann)
include(rectpack2d)
include(wuffs)
include(fpng)
include(argparse)
include(GoogleTest)

setup_nlohmann_json()
setup_rectpack2d()
setup_wuffs()
setup_fpng()
setup_gtest()

setup_argparse()

add_library(TiredPackerLib
    src/AtlasUtils.cpp
    src/ImageFile.cpp
    src/AtlasExplorer.cpp
    src/WuffsImplementation.cpp
)

target_link_libraries(TiredPackerLib PUBLIC
    wuffs
    FPNG::FPNG
    rectpack2D
    nlohmann_json::nlohmann_json
)

target_compile_definitions(TiredPackerLib PRIVATE
    ATLAS_METADATA_VERSION=${ATLAS_METADATA_VERSION})

add_executable(TiredPacker 
src/main.cpp
)

target_link_libraries(TiredPacker PRIVATE TiredPackerLib
    argparse::argparse)


add_subdirectory(tests)
