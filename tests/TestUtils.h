#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include <fpng.h>
#include <gtest/gtest.h>

namespace TestUtils {
class ScopedTestDir {
public:
    explicit ScopedTestDir(const std::string& testName)
        : mPath(CreateTempTestDir(testName)) {
    }

    ~ScopedTestDir() {
        std::filesystem::remove_all(mPath);
    }

    const std::filesystem::path& path() const noexcept {
        return mPath;
    }

private:
    static std::filesystem::path
    CreateTempTestDir(const std::string& testName) {
        const auto dir =
            std::filesystem::temp_directory_path() /
            ("tiredpacker_" + testName + "_" +
             std::to_string(
                 std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(dir);
        return dir;
    }

    std::filesystem::path mPath;
};

inline std::string FileTimestampString(const std::filesystem::path& path) {
    const auto timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::filesystem::last_write_time(path).time_since_epoch())
            .count();
    return std::to_string(timestamp);
}

inline void WritePngFile(const std::filesystem::path& path, uint32_t width,
                         uint32_t height,
                         const std::vector<uint8_t>& rgbaPixels) {
    EXPECT_TRUE(fpng::fpng_encode_image_to_file(
        path.string().c_str(), rgbaPixels.data(), width, height, 4));
}
} // namespace TestUtils
