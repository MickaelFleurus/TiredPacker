
enable_testing()

add_executable(TiredPackerTests
    MetadataTests.cpp
    ImageFileTests.cpp
    AtlasGenerationTests.cpp
)

target_link_libraries(TiredPackerTests PRIVATE
    TiredPackerLib
    GTest::gtest_main
)

target_include_directories(TiredPackerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

add_test(NAME TiredPackerTests COMMAND TiredPackerTests)
