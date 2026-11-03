#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "AtlasReader.h"
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
    EXPECT_EQ(metadata["atlases"][0]["file_name"], "spritepack_0.png");

    const auto parsed = AtlasReader::ParseMetadata(metadataPath);
    ASSERT_EQ(parsed.version, 1);
    ASSERT_EQ(parsed.atlases.size(), 1);
    EXPECT_EQ(parsed.atlases[0].fileName, "spritepack_0.png");
    EXPECT_GE(parsed.atlases[0].width, 4);
    EXPECT_GE(parsed.atlases[0].height, 4);

    const auto found = AtlasReader::FindEntry(parsed, "a.png");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->entry->fileName, "a.png");
    EXPECT_EQ(found->entry->width, 1);
    EXPECT_EQ(found->atlas->fileName, "spritepack_0.png");
    EXPECT_FALSE(AtlasReader::FindEntry(parsed, "missing.png").has_value());
}

TEST(AtlasGenerationTests, RepackedAtlasBoundsIncludePlacementOffset) {
    TestUtils::ScopedTestDir tempDir("atlas_padding_bounds");
    const auto outputFolder = tempDir.path() / "output";
    std::filesystem::create_directories(outputFolder);

    const auto imagePath = tempDir.path() / "boundary.png";
    TestUtils::WritePngFile(imagePath, 1024, 1024,
                            std::vector<uint8_t>(1024 * 1024 * 4, 255));

    std::map<std::string, CImageFile> files;
    files.emplace("boundary.png", CImageFile(imagePath));
    Utility::Generate(outputFolder, "boundary", files);

    std::ifstream jsonIn(outputFolder / "boundary_metadata.json");
    nlohmann::json metadata;
    jsonIn >> metadata;

    const auto& atlas = metadata["atlases"][0];
    const auto& entry = atlas["entries"][0];
    EXPECT_GE(atlas["width"].get<int>(),
              entry["x"].get<int>() + entry["width"].get<int>());
    EXPECT_GE(atlas["height"].get<int>(),
              entry["y"].get<int>() + entry["height"].get<int>());
}
