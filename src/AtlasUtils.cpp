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
};

struct SAtlasInfo {
    int w = 0, h = 0;
};

struct SMetadataEntry {
    uint64_t fileSize;
    std::string lastModified;
};

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

std::vector<SPlacement> Pack(const std::vector<rectpack2D::rect_wh>& sizes,
                             const std::vector<std::string>& fileNames,
                             std::vector<SAtlasInfo>& atlasesOut) {
    using namespace rectpack2D;
    constexpr bool allow_flip = false;
    using spaces_type = empty_spaces<allow_flip, default_empty_spaces>;
    using rect_type = output_rect_t<spaces_type>;

    constexpr int max_side = 2048;
    constexpr int discard_step = 1;

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
        spaces_type bin(rect_wh(max_side, max_side));
        std::vector<std::size_t> leftover;
        SAtlasInfo info;
        const int atlas_index = (int)atlasesOut.size();

        for (std::size_t id : pending) {
            if (const auto placed = bin.insert(sizes[id])) {
                out[id] = {atlas_index, fileNames[id], *placed};
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

std::expected<std::map<std::string, SMetadataEntry>, std::string>
ParseJsonFile(const std::filesystem::path& filePath) {
    using json = nlohmann::json;

    std::map<std::string, SMetadataEntry> datas;
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

    for (const auto& atlasJson : j["atlases"]) {

        if (!atlasJson.contains("entries")) {
            continue;
        }

        for (const auto& entryJson : atlasJson["entries"]) {
            SMetadataEntry entry;
            entry.fileSize = entryJson.value("fileSize", 0ULL);
            entry.lastModified = entryJson.value("lastModified", "");
            datas.emplace(entryJson.value("file", std::string()), entry);
        }
    }

    return datas;
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
    const auto& jsonData = expectedData.value();
    if (files.size() != jsonData.size()) {
        return true;
    }
    for (const auto& data : jsonData) {

        for (size_t i = 0; i < files.size(); ++i) {
            const auto& file = files.at(data.first);
            const auto& entry = jsonData.at(data.first);
            if (file.fileSize() != entry.fileSize ||
                file.lastModified() != entry.lastModified) {
                return true;
            }
        }
    }

    return false;
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
    // Save the json metadata file
    nlohmann::json j;
    j["atlases"] = nlohmann::json::array();
    std::size_t atlasIndex = 0;
    std::vector<std::vector<uint8_t>> atlasPixels;
    for (const auto& atlas : atlases) {
        std::size_t width = nextPowerOfTwo(atlas.w);
        std::size_t height = nextPowerOfTwo(atlas.h);
        nlohmann::json atlas_json;
        atlas_json["width"] = width;
        atlas_json["height"] = height;
        atlas_json["file_name"] = std::format(
            "{}_{}.png", outputFolder.filename().string(), atlasIndex++);
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

        const std::size_t atlasWidth = static_cast<std::size_t>(
            nextPowerOfTwo(atlases[placement.atlas].w));
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
        const uint32_t width = nextPowerOfTwo(atlas.w);
        const uint32_t height = nextPowerOfTwo(atlas.h);

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
