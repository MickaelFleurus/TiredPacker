cmake_minimum_required(VERSION 4.4)
project(TiredPacker VERSION 1.0 LANGUAGES CXX)
include(FetchContent)

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
include(PNGLib)

setup_nlohmann_json()
setup_rectpack2d()
setup_pnglib()

add_executable(TiredPacker 
src/main.cpp
src/SpritePacker.cpp
src/PngImage.cpp)

target_link_libraries(TiredPacker PRIVATE PNG::PNG rectpack2D::rectpack2D nlohmann_json::nlohmann_json)

