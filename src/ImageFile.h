#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class CImageFile {
public:
    CImageFile(std::filesystem::path imagePath);
    bool Load();

    uint32_t width() const noexcept {
        return mWidth;
    }
    uint32_t height() const noexcept {
        return mHeight;
    }
    const std::vector<uint8_t>& pixels() const noexcept {
        return mPixels;
    }

    std::string fileName() const noexcept {
        return mPath.filename().string();
    }
    uint64_t fileSize() const noexcept;
    std::string lastModified() const noexcept;

private:
    std::filesystem::path mPath;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    std::vector<uint8_t> mPixels;
};
