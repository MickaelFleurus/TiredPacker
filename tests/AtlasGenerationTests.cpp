#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "AtlasUtils.h"
#include "ImageFile.h"
#include "TestUtils.h"

TEST(AtlasGenerationTests, GenerateWritesAtlasAndMetadata) {
    TestUtils::ScopedTestDir tempDir("generate_atlas");
    const auto outputFolder = tempDir.path() / "output";
    std::filesystem::create_directories(outputFolder);

    const auto imageA = tempDir.path() / "a.png";
    const auto imageB = tempDir.path() / "b.png";
    TestUtils::WritePngFile(imageA, 1, 1, {255, 0, 0, 255});
    TestUtils::WritePngFile(
        imageB, 2, 2,
        {0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 0, 255, 255, 0, 255, 255});

    std::map<std::string, CImageFile> files;
    files.emplace("a.png", CImageFile(imageA));
    files.emplace("b.png", CImageFile(imageB));

    Utility::Generate(outputFolder, "spritepack", files);

    const auto atlasPath = outputFolder / "spritepack_0.png";
    const auto metadataPath = outputFolder / "spritepack_metadata.json";
    EXPECT_TRUE(std::filesystem::exists(atlasPath));
    EXPECT_TRUE(std::filesystem::exists(metadataPath));

    std::ifstream jsonIn(metadataPath);
    nlohmann::json metadata;
    jsonIn >> metadata;
    EXPECT_EQ(metadata["version"], 1);
    EXPECT_EQ(metadata["atlases"].size(), 1);
    EXPECT_EQ(metadata["atlases"][0]["entries"].size(), 2);
}
