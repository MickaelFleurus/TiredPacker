#include "GeneratedAtlas.h"

#include <algorithm>
#include <fstream>
#include <numeric>

#include <fpng.h>
#include <nlohmann/json.hpp>
#include <rectpack2D/finders_interface.h>

#include "ImageFile.h"

namespace {
struct Placement {
    int atlas;
    std::size_t imgId;
    rectpack2D::rect_xywh rect;
};

struct AtlasInfo {
    int w = 0, h = 0;
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

std::vector<Placement>
pack_atlases(const std::vector<rectpack2D::rect_wh>& sizes,
             std::vector<AtlasInfo>& atlases) {
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

    std::vector<Placement> out(sizes.size());
    atlases.clear();

    while (!pending.empty()) {
        spaces_type bin(rect_wh(max_side, max_side));
        std::vector<std::size_t> leftover;
        AtlasInfo info;
        const int atlas_index = (int)atlases.size();

        for (std::size_t id : pending) {
            if (const auto placed = bin.insert(sizes[id])) {
                out[id] = {atlas_index, id, *placed};
                info.w = std::max(info.w, placed->x + placed->w);
                info.h = std::max(info.h, placed->y + placed->h);
            } else {
                leftover.push_back(id);
            }
        }

        atlases.push_back(info); // tight bounds of what was actually placed
        pending = std::move(leftover);
    }
    return out;
}
} // namespace

CGeneratedAtlas::CGeneratedAtlas(std::filesystem::path atlasFolder)
    : mAtlasFolder(atlasFolder) {
    constexpr std::array<std::string_view, 2> validExtensions = {".png",
                                                                 ".jpg"};
    for (const auto& entry :
         std::filesystem::directory_iterator(mAtlasFolder)) {
        if (entry.is_regular_file() &&
            std::any_of(validExtensions.begin(), validExtensions.end(),
                        [&](const std::string_view& ext) {
                            return entry.path().extension() == ext;
                        })) {
            mFiles.emplace_back(entry.path());
        }
    }
}

const std::vector<CImageFile>& CGeneratedAtlas::GetFiles() const {
    return mFiles;
}

void CGeneratedAtlas::TryGenerate() {
    using namespace rectpack2D;

    std::vector<rect_wh> rects;
    rects.reserve(mFiles.size());
    for (auto& file : mFiles) {
        file.Load();
        rects.emplace_back(rect_wh(file.width(), file.height()));
    }

    std::vector<AtlasInfo> atlases;
    auto placements = pack_atlases(rects, atlases);
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
            "{}_{}.png", mAtlasFolder.filename().string(), atlasIndex++);
        atlas_json["entries"] = nlohmann::json::array();
        j["atlases"].push_back(atlas_json);

        atlasPixels.emplace_back(width * height * 4, 0);
    }
    for (const auto& placement : placements) {
        const auto& file = mFiles[placement.imgId];
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
    fpng::fpng_init();
    for (std::size_t i = 0; i < atlases.size(); ++i) {
        const auto& atlas = atlases[i];
        const uint32_t width = nextPowerOfTwo(atlas.w);
        const uint32_t height = nextPowerOfTwo(atlas.h);

        const auto outputPath =
            (mAtlasFolder / std::format("{}_{}.png",
                                        mAtlasFolder.filename().string(),
                                        atlasIndex++))
                .string();
        if (!fpng::fpng_encode_image_to_file(
                outputPath.c_str(), atlasPixels[i].data(), width, height, 4)) {
            // Handle encoding failure.
        }
    }

    std::ofstream metadataFile(mAtlasFolder / "atlas_metadata.json");
    metadataFile << j.dump(4);
    metadataFile.close();
}
