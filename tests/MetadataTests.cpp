#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>

#include "AtlasUtils.h"
#include "ImageFile.h"
#include "TestUtils.h"

TEST(MetadataTests, ValidMetadataReturnsTrue) {
    TestUtils::ScopedTestDir tempDir("valid_metadata");
    const auto assetPath = tempDir.path() / "sprite.png";

    std::vector<uint8_t> pixels = {
        255, 0, 0, 255, 0, 255, 0, 255,
    };
    TestUtils::WritePngFile(assetPath, 2, 1, pixels);

    std::map<std::string, CImageFile> files;
    files.emplace(assetPath.filename().string(), CImageFile(assetPath));

    const auto metadataPath = tempDir.path() / "atlas_metadata.json";
    std::ofstream metadata(metadataPath);
    metadata << "{\n"
             << "  \"version\": 1,\n"
             << "  \"atlases\": [\n"
             << "    {\n"
             << "      \"entries\": [\n"
             << "        {\n"
             << "          \"file\": \"sprite.png\",\n"
             << "          \"fileSize\": "
             << std::filesystem::file_size(assetPath) << ",\n"
             << "          \"lastModified\": \""
             << TestUtils::FileTimestampString(assetPath) << "\"\n"
             << "        }\n"
             << "      ]\n"
             << "    }\n"
             << "  ]\n"
             << "}\n";
    metadata.close();

    EXPECT_TRUE(Utility::AreMetadataValid(metadataPath, files));
}

TEST(MetadataTests, InvalidMetadataReturnsFalseWhenFileChanged) {
    TestUtils::ScopedTestDir tempDir("invalid_metadata");
    const auto assetPath = tempDir.path() / "sprite.png";

    std::vector<uint8_t> pixels = {
        255, 0, 0, 255, 0, 255, 0, 255,
    };
    TestUtils::WritePngFile(assetPath, 2, 1, pixels);

    std::map<std::string, CImageFile> files;
    files.emplace(assetPath.filename().string(), CImageFile(assetPath));

    const auto metadataPath = tempDir.path() / "atlas_metadata.json";
    std::ofstream metadata(metadataPath);
    metadata << "{\n"
             << "  \"version\": 1,\n"
             << "  \"atlases\": [\n"
             << "    {\n"
             << "      \"entries\": [\n"
             << "        {\n"
             << "          \"file\": \"sprite.png\",\n"
             << "          \"fileSize\": 999999,\n"
             << "          \"lastModified\": \"0\"\n"
             << "        }\n"
             << "      ]\n"
             << "    }\n"
             << "  ]\n"
             << "}\n";
    metadata.close();

    EXPECT_FALSE(Utility::AreMetadataValid(metadataPath, files));
}

TEST(MetadataTests, GatherFilesOnlyReturnsSupportedImages) {
    TestUtils::ScopedTestDir tempDir("gather_files");

    {
        std::ofstream(tempDir.path() / "a.png") << "png";
        std::ofstream(tempDir.path() / "b.jpg") << "jpg";
        std::ofstream(tempDir.path() / "c.txt") << "text";
    }

    const auto files = Utility::GatherFiles(tempDir.path());

    EXPECT_EQ(files.size(), 2u);
    EXPECT_TRUE(files.contains("a.png"));
    EXPECT_TRUE(files.contains("b.jpg"));
    EXPECT_FALSE(files.contains("c.txt"));
}
