#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace tired {

class PngImage {
public:
    PngImage() = default;
    PngImage(unsigned width, unsigned height, std::vector<std::uint8_t> pixels);

    static PngImage load(const std::filesystem::path& path);
    void save(const std::filesystem::path& path) const;

    unsigned width() const noexcept { return m_width; }
    unsigned height() const noexcept { return m_height; }
    const std::vector<std::uint8_t>& pixels() const noexcept { return m_pixels; }
    std::vector<std::uint8_t>& pixels() noexcept { return m_pixels; }

private:
    unsigned m_width = 0;
    unsigned m_height = 0;
    std::vector<std::uint8_t> m_pixels;
};

} // namespace tired
