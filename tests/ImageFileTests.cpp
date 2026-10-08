#include <vector>

#include <gtest/gtest.h>

#include "ImageFile.h"
#include "TestUtils.h"

TEST(ImageFileTests, LoadReadsRealPngPixels) {
    TestUtils::ScopedTestDir tempDir("image_load");
    const auto assetPath = tempDir.path() / "pixel_test.png";

    std::vector<uint8_t> pixels = {
        255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 0, 255,
    };
    TestUtils::WritePngFile(assetPath, 2, 2, pixels);

    CImageFile image(assetPath);
    image.Load();

    EXPECT_EQ(image.width(), 2u);
    EXPECT_EQ(image.height(), 2u);
    EXPECT_EQ(image.pixels().size(), 2u * 2u * 4u);
    EXPECT_EQ(image.pixels()[0], 255u);
    EXPECT_EQ(image.pixels()[1], 0u);
    EXPECT_EQ(image.pixels()[2], 0u);
    EXPECT_EQ(image.pixels()[3], 255u);
    EXPECT_EQ(image.pixels()[4], 0u);
    EXPECT_EQ(image.pixels()[5], 255u);
    EXPECT_EQ(image.pixels()[6], 0u);
    EXPECT_EQ(image.pixels()[7], 255u);
}
