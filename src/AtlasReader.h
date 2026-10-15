#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace AtlasReader {

struct AtlasEntry {
    std::string fileName;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t uvU = 0;
    uint32_t uvV = 0;
    uint64_t fileSize = 0;
    std::string lastModified;
};

struct Atlas {
    std::string fileName;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<AtlasEntry> entries;
};

struct Metadata {
    uint32_t version = 0;
    std::string name;
    std::vector<Atlas> atlases;
};

struct AtlasLocation {
    const Atlas* atlas;
    const AtlasEntry* entry;
};

Metadata ParseMetadata(const std::filesystem::path& metadataPath);
std::optional<AtlasLocation> FindEntry(const Metadata& metadata,
                                       std::string_view fileName) noexcept;

} // namespace AtlasReader
