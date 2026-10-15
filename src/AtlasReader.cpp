#include "AtlasReader.h"

#include <format>
#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace AtlasReader {
namespace {

constexpr uint32_t kSupportedMetadataVersion = 2;

} // namespace

Metadata ParseMetadata(const std::filesystem::path& metadataPath) {
    std::ifstream input(metadataPath);
    if (!input.is_open()) {
        throw std::runtime_error(std::format(
            "Failed to open atlas metadata: {}", metadataPath.string()));
    }

    try {
        const auto document = nlohmann::json::parse(input);
        Metadata metadata;
        metadata.name = document.at("name").get<std::string>();
        metadata.version = document.at("version").get<uint32_t>();
        if (metadata.version != kSupportedMetadataVersion) {
            throw std::runtime_error(std::format(
                "Unsupported atlas metadata version: {}", metadata.version));
        }

        for (const auto& atlasJson : document.at("atlases")) {
            Atlas atlas;
            atlas.fileName = atlasJson.at("file_name").get<std::string>();
            atlas.width = atlasJson.at("width").get<uint32_t>();
            atlas.height = atlasJson.at("height").get<uint32_t>();
            if (atlas.fileName.empty() || atlas.width == 0 ||
                atlas.height == 0) {
                throw std::runtime_error(
                    "Atlas has an invalid filename or size");
            }

            for (const auto& entryJson : atlasJson.at("entries")) {
                AtlasEntry entry;
                entry.fileName = entryJson.at("file").get<std::string>();
                entry.width = entryJson.at("width").get<uint32_t>();
                entry.height = entryJson.at("height").get<uint32_t>();
                entry.x = entryJson.at("x").get<uint32_t>();
                entry.y = entryJson.at("y").get<uint32_t>();
                entry.uvU = entryJson.at("uv_u").get<uint32_t>();
                entry.uvV = entryJson.at("uv_v").get<uint32_t>();
                entry.fileSize = entryJson.at("fileSize").get<uint64_t>();
                entry.lastModified =
                    entryJson.at("lastModified").get<std::string>();

                if (entry.fileName.empty() || entry.width == 0 ||
                    entry.height == 0 ||
                    entry.x + uint64_t{entry.width} > atlas.width ||
                    entry.y + uint64_t{entry.height} > atlas.height) {
                    throw std::runtime_error(
                        "Atlas entry has invalid bounds or dimensions");
                }
                atlas.entries.push_back(std::move(entry));
            }
            metadata.atlases.push_back(std::move(atlas));
        }
        return metadata;
    } catch (const std::exception& error) {
        throw std::runtime_error(
            std::format("Failed to parse atlas metadata '{}': {}",
                        metadataPath.string(), error.what()));
    }
}

std::optional<AtlasLocation> FindEntry(const Metadata& metadata,
                                       std::string_view fileName) noexcept {
    for (const auto& atlas : metadata.atlases) {
        for (const auto& entry : atlas.entries) {
            if (entry.fileName == fileName) {
                return AtlasLocation{&atlas, &entry};
            }
        }
    }
    return std::nullopt;
}

} // namespace AtlasReader
