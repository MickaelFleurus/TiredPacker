#include "ImageFile.h"

#include <cstring>
#include <format>
#include <fstream>
#include <vector>

#include "WuffsInclude.h"

namespace {

class RgbaDecodeCallbacks : public wuffs_aux::DecodeImageCallbacks {
public:
    wuffs_base__pixel_format
    SelectPixfmt(const wuffs_base__image_config&) override {
        return wuffs_base__make_pixel_format(
            WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL);
    }
};

} // namespace

CImageFile::CImageFile(std::filesystem::path imagePath) : mPath(imagePath) {
}

uint32_t CImageFile::width() const noexcept {
    return mWidth;
}

uint32_t CImageFile::height() const noexcept {
    return mHeight;
}

const std::vector<uint8_t>& CImageFile::pixels() const noexcept {
    return mPixels;
}

std::string CImageFile::fileName() const noexcept {
    return mPath.filename().string();
}

uint64_t CImageFile::fileSize() const noexcept {
    return std::filesystem::file_size(mPath);
}

std::string CImageFile::lastModified() const noexcept {
    const auto timestamp =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::filesystem::last_write_time(mPath).time_since_epoch())
            .count();
    return std::format("{}", timestamp);
}

void CImageFile::Load() {

    std::ifstream file(mPath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error(
            std::format("Failed to open image file: {}", mPath.string()));
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error(
            std::format("Failed to read image file: {}", mPath.string()));
    }
    file.close();

    // 2. Wrap buffer using Wuffs' synchronous memory sync input
    wuffs_aux::sync_io::MemoryInput input(buffer.data(), buffer.size());

    // 3. Decode the image into a standard pixel buffer
    RgbaDecodeCallbacks callbacks;
    wuffs_aux::DecodeImageResult result =
        wuffs_aux::DecodeImage(callbacks, input);

    if (!result.error_message.empty()) {
        throw std::runtime_error(
            std::format("Failed to decode image file[] : {}", mPath.string(),
                        result.error_message));
    }

    mWidth = result.pixbuf.pixcfg.width();
    mHeight = result.pixbuf.pixcfg.height();
    const wuffs_base__table_u8 plane = result.pixbuf.plane(0);
    if (plane.width != mWidth * 4 || plane.height != mHeight ||
        plane.stride < plane.width) {
        throw std::runtime_error(std::format(
            "Invalid image dimensions in file: {}", mPath.string()));
    }

    mPixels.resize(static_cast<std::size_t>(mWidth) * mHeight * 4);
    for (uint32_t y = 0; y < mHeight; ++y) {
        std::memcpy(mPixels.data() + static_cast<std::size_t>(y) * plane.width,
                    plane.ptr + static_cast<std::size_t>(y) * plane.stride,
                    plane.width);
    }
}
