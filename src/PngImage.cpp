#include "PngImage.h"

#include <png.h>

#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

PngImage::PngImage(unsigned width, unsigned height, std::vector<uint8_t> pixels)
    : m_width(width), m_height(height), m_pixels(std::move(pixels)) {
  if (m_pixels.size() != static_cast<std::size_t>(width) * height * 4) {
    throw std::invalid_argument("RGBA pixel buffer has an invalid size");
  }
}

PngImage PngImage::load(const std::filesystem::path &path) {
  FILE *file = std::fopen(path.string().c_str(), "rb");
  if (!file)
    throw std::runtime_error("Could not open PNG: " + path.string());

  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop info = png_create_info_struct(png);
  if (!png || !info) {
    if (png)
      png_destroy_read_struct(&png, nullptr, nullptr);
    std::fclose(file);
    throw std::runtime_error("Could not initialize libpng");
  }
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_read_struct(&png, &info, nullptr);
    std::fclose(file);
    throw std::runtime_error("Could not decode PNG: " + path.string());
  }

  png_init_io(png, file);
  png_read_info(png, info);
  const auto width = png_get_image_width(png, info);
  const auto height = png_get_image_height(png, info);
  const auto color = png_get_color_type(png, info);
  const auto depth = png_get_bit_depth(png, info);

  if (depth == 16)
    png_set_strip_16(png);
  if (color == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  if (color == PNG_COLOR_TYPE_GRAY && depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  if (color == PNG_COLOR_TYPE_RGB || color == PNG_COLOR_TYPE_GRAY ||
      color == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);
  std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
  std::vector<png_bytep> rows(height);
  for (unsigned y = 0; y < height; ++y)
    rows[y] = pixels.data() + static_cast<std::size_t>(y) * width * 4;
  png_read_image(png, rows.data());
  png_read_end(png, nullptr);
  png_destroy_read_struct(&png, &info, nullptr);
  std::fclose(file);
  return PngImage(width, height, std::move(pixels));
}

void PngImage::save(const std::filesystem::path &path) const {
  FILE *file = std::fopen(path.string().c_str(), "wb");
  if (!file)
    throw std::runtime_error("Could not create PNG: " + path.string());
  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  png_infop info = png_create_info_struct(png);
  if (!png || !info) {
    if (png)
      png_destroy_write_struct(&png, nullptr);
    std::fclose(file);
    throw std::runtime_error("Could not initialize libpng");
  }
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_write_struct(&png, &info);
    std::fclose(file);
    throw std::runtime_error("Could not encode PNG: " + path.string());
  }
  png_init_io(png, file);
  png_set_IHDR(png, info, m_width, m_height, 8, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  std::vector<png_bytep> rows(m_height);
  for (unsigned y = 0; y < m_height; ++y)
    rows[y] = const_cast<png_bytep>(m_pixels.data() +
                                    static_cast<std::size_t>(y) * m_width * 4);
  png_write_image(png, rows.data());
  png_write_end(png, nullptr);
  png_destroy_write_struct(&png, &info);
  std::fclose(file);
}

