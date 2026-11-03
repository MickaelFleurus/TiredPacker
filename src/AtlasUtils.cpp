#include "AtlasUtils.h"

#include <algorithm>
#include <expected>
#include <fstream>
#include <numeric>

#include <fpng.h>
#include <nlohmann/json.hpp>
#include <rectpack2D/finders_interface.h>

namespace {
struct SPlacement {
    int atlas;
    std::string fileName;
    rectpack2D::rect_xywh rect;
    int padding;
};

struct SAtlasInfo {
    int w = 0, h = 0;
};

struct SMetadataEntry {
    uint64_t fileSize;
    std::string lastModified;
};

struct SAtlasMetadata {
    int version = 0;
    std::string name;
    std::map<std::string, SMetadataEntry> entries;
};

constexpr int kAtlasMetadataVersion = ATLAS_METADATA_VERSION;

using namespace rectpack2D;
constexpr bool kAllowFlip = false;
using spaces_type = empty_spaces<kAllowFlip, default_empty_spaces>;
using rect_type = output_rect_t<spaces_type>;
constexpr int kMaxSize = 2048;
constexpr int kPadding = 1;
constexpr int kDiscardStep = 1;

constexpr int nextPowerOfTwo(int value) {
    if (value <= 1) {
        return 1;
    }

    int power = 1;
    while (power < value) {
        power <<= 1;
    }
    return power;
}

void TightenPlacements(std::vector<SPlacement>& placements,
                       std::vector<SAtlasInfo>& atlases) {

    for (int a = 0; a < (int)atlases.size(); ++a) {
        // Gather this atlas's placements
        std::vector<SPlacement*> group;
        std::vector<rect_type> rects;
        for (auto& p : placements) {
            if (p.atlas == a) {
                rects.push_back(p.rect);
                rects.back().w += 2 * p.padding;
                rects.back().h += 2 * p.padding;
                group.push_back(&p);
            }
        }

        const auto size = rectpack2D::find_best_packing<spaces_type>(
            rects, rectpack2D::make_finder_input(
                       kMaxSize, kDiscardStep,
                       [](rect_type&) {
                           return rectpack2D::callback_result::CONTINUE_PACKING;
                       },
                       [](rect_type&) {
                           return rectpack2D::callback_result::ABORT_PACKING;
                       },
                       rectpack2D::flipping_option::DISABLED));

        if (size.w == 0 || size.h == 0)
            continue;

        for (size_t i = 0; i < group.size(); ++i) {
            group[i]->rect.x = rects[i].x + group[i]->padding;
            group[i]->rect.y = rects[i].y + group[i]->padding;
        }

        atlases[a].w = nextPowerOfTwo(size.w);
        atlases[a].h = nextPowerOfTwo(size.h);
    }
}

std::vector<SPlacement> Pack(const std::vector<rectpack2D::rect_wh>& sizes,
                             const std::vector<std::string>& fileNames,
                             std::vector<SAtlasInfo>& atlasesOut) {

    std::vector<std::size_t> pending(sizes.size());
    std::iota(pending.begin(), pending.end(), 0);

    // largest first
    std::sort(pending.begin(), pending.end(),
              [&](std::size_t a, std::size_t b) {
                  return sizes[a].w * sizes[a].h > sizes[b].w * sizes[b].h;
              });

    std::vector<SPlacement> out(sizes.size());
    atlasesOut.clear();

    while (!pending.empty()) {
        spaces_type bin(rect_wh(kMaxSize, kMaxSize));
        std::vector<std::size_t> leftover;
        SAtlasInfo info;
        const int atlas_index = (int)atlasesOut.size();

        for (std::size_t id : pending) {
            const int padding =
                sizes[id].w + 2 * kPadding <= kMaxSize &&
                        sizes[id].h + 2 * kPadding <= kMaxSize
                    ? kPadding
                    : 0;
            const rect_wh packedSize(sizes[id].w + 2 * padding,
                                     sizes[id].h + 2 * padding);
            if (const auto placed = bin.insert(packedSize)) {
                rect_xywh imageRect = *placed;
                imageRect.x += padding;
                imageRect.y += padding;
                imageRect.w = sizes[id].w;
                imageRect.h = sizes[id].h;
                out[id] = {atlas_index, fileNames[id], imageRect, padding};
                info.w = std::max(info.w, placed->x + placed->w);
                info.h = std::max(info.h, placed->y + placed->h);
            } else {
                leftover.push_back(id);
            }
        }

        atlasesOut.push_back(info);
        pending = std::move(leftover);
    }

    return out;
}

std::expected<SAtlasMetadata, std::string>
ParseJsonFile(const std::filesystem::path& filePath) {
    using json = nlohmann::json;

    SAtlasMetadata metadata;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::unexpected<std::string>(
            std::format("Failed to open metadata file: {}", filePath.string()));
    }

    json j;
    file >> j;
    if (!j.contains("atlases")) {
        return std::unexpected<std::string>(
            std::format("Metadata file does not contain 'atlases' section: {}",
                        filePath.string()));
    }
    metadata.version = j.value("version", 0);
    metadata.name = j.value("name", "default_name");

    for (const auto& atlasJson : j["atlases"]) {

        if (!atlasJson.contains("entries")) {
            continue;
        }

        for (const auto& entryJson : atlasJson["entries"]) {
            SMetadataEntry entry;
            entry.fileSize = entryJson.value("fileSize", 0ULL);
            entry.lastModified = entryJson.value("lastModified", "");
            metadata.entries.emplace(entryJson.value("file", std::string()),
                                     entry);
        }
    }

    return metadata;
}
} // namespace

namespace Utility {

bool AreMetadataValid(const std::filesystem::path& metadataFilePath,
                      const std::map<std::string, CImageFile>& files) {

    const auto expectedData = ParseJsonFile(metadataFilePath);
    if (!expectedData.has_value()) {
        throw std::runtime_error(
            std::format("Failed to parse metadata file: {}. Error: {}. "
                        "Removing the file and regerating it.",
                        metadataFilePath.string(), expectedData.error()));
    }
    const auto& metadata = expectedData.value();
    if (metadata.version != kAtlasMetadataVersion ||
        files.size() != metadata.entries.size()) {
        return false;
    }
    for (const auto& [fileName, entry] : metadata.entries) {
        const auto file = files.find(fileName);
        if (file == files.end() || file->second.fileSize() != entry.fileSize ||
            file->second.lastModified() != entry.lastModified) {
            return false;
        }
    }

    return true;
}

std::map<std::string, CImageFile>
GatherFiles(const std::filesystem::path& assetsFolder) {
    std::map<std::string, CImageFile> files;
    constexpr std::array<std::string_view, 2> validExtensions = {".png",
                                                                 ".jpg"};
    for (const auto& entry :
         std::filesystem::directory_iterator(assetsFolder)) {
        if (entry.is_regular_file() &&
            std::any_of(validExtensions.begin(), validExtensions.end(),
                        [&](const std::string_view& ext) {
                            return entry.path().extension() == ext;
                        })) {
            files.try_emplace(entry.path().filename().string(), entry.path());
        }
    }
    return files;
}

void Generate(const std::filesystem::path& outputFolder,
              const std::string& name,
              std::map<std::string, CImageFile>& files) {
    using namespace rectpack2D;

    std::vector<rect_wh> rects;
    rects.reserve(files.size());
    std::vector<std::string> fileNames;
    fileNames.reserve(files.size());
    for (auto& [fileName, file] : files) {
        file.Load();
        rects.emplace_back(rect_wh(file.width(), file.height()));
        fileNames.push_back(fileName);
    }

    std::vector<SAtlasInfo> atlases;
    auto placements = Pack(rects, fileNames, atlases);
    TightenPlacements(placements, atlases);
    // Save the json metadata file
    nlohmann::json j;
    j["version"] = kAtlasMetadataVersion;
    j["name"] = name;
    j["atlases"] = nlohmann::json::array();
    std::size_t atlasIndex = 0;
    std::vector<std::vector<uint8_t>> atlasPixels;
    for (const auto& atlas : atlases) {
        std::size_t width = atlas.w;
        std::size_t height = atlas.h;
        nlohmann::json atlas_json;
        atlas_json["width"] = width;
        atlas_json["height"] = height;
        atlas_json["file_name"] = std::format("{}_{}.png", name, atlasIndex++);
        atlas_json["entries"] = nlohmann::json::array();
        j["atlases"].push_back(atlas_json);

        atlasPixels.emplace_back(width * height * 4, 0);
    }
    for (const auto& placement : placements) {
        const auto& file = files.at(placement.fileName);
        nlohmann::json entry;
        entry["file"] = file.fileName();
        entry["width"] = file.width();
        entry["height"] = file.height();
        entry["x"] = placement.rect.x;
        entry["y"] = placement.rect.y;
        entry["uv_u"] = placement.rect.w;
        entry["uv_v"] = placement.rect.h;
        entry["fileSize"] = file.fileSize();
        entry["lastModified"] = file.lastModified();
        j["atlases"][placement.atlas]["entries"].push_back(entry);

        const auto& pixels = file.pixels();
        auto& atlas = atlasPixels[placement.atlas];

        const std::size_t atlasWidth =
            static_cast<std::size_t>(atlases[placement.atlas].w);
        const std::size_t rowBytes = static_cast<std::size_t>(file.width()) * 4;
        for (uint32_t row = 0; row < file.height(); ++row) {
            const std::size_t destinationOffset =
                (static_cast<std::size_t>(placement.rect.y + row) * atlasWidth +
                 placement.rect.x) *
                4;

            std::copy_n(pixels.data() +
                            static_cast<std::size_t>(row) * rowBytes,
                        rowBytes, atlas.data() + destinationOffset);
        }
    }
    atlasIndex = 0;
    for (std::size_t i = 0; i < atlases.size(); ++i) {
        const auto& atlas = atlases[i];
        const uint32_t width = atlas.w;
        const uint32_t height = atlas.h;

        const auto outputPath =
            (outputFolder / std::format("{}_{}.png", name, atlasIndex++))
                .string();
        if (!fpng::fpng_encode_image_to_file(
                outputPath.c_str(), atlasPixels[i].data(), width, height, 4)) {
            throw std::runtime_error(
                std::format("Failed to write atlas image: {}", outputPath));
        }
    }
    const auto metadataFile =
        (outputFolder / std::format("{}_metadata.json", name)).string();
    std::ofstream jsonOut(metadataFile);
    jsonOut << j.dump(4);
    jsonOut.close();
}
} // namespace Utility
