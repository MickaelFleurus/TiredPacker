#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class CImageFile {
public:
    CImageFile(std::filesystem::path imagePath);
    void Load();

    uint32_t width() const noexcept;
    uint32_t height() const noexcept;
    const std::vector<uint8_t>& pixels() const noexcept;

    std::string fileName() const noexcept;
    uint64_t fileSize() const noexcept;
    std::string lastModified() const noexcept;

private:
    std::filesystem::path mPath;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    std::vector<uint8_t> mPixels;
};
