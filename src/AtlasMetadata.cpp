#include "AtlasMetadata.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace {
using json = nlohmann::json;
std::vector<CAtlasMetadata::SData>
ParseJsonFile(const std::filesystem::path& filePath) {
    std::vector<CAtlasMetadata::SData> datas;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return datas;
    }
    json j;
    file >> j;
    if (j.contains("atlases")) {
        for (const auto& atlasJson : j["atlases"]) {
            CAtlasMetadata::SData data;
            if (atlasJson.contains("entries")) {
                for (const auto& entryJson : atlasJson["entries"]) {
                    CAtlasMetadata::SEntry entry;
                    entry.width = entryJson.value("width", 0);
                    entry.height = entryJson.value("height", 0);
                    entry.x = entryJson.value("x", 0);
                    entry.y = entryJson.value("y", 0);
                    entry.fileSize = entryJson.value("fileSize", 0ULL);
                    entry.lastModified = entryJson.value("lastModified", "");
                    data.entries.push_back(entry);
                }
            }
        }
    }
    return datas;
}
} // namespace

CAtlasMetadata::CAtlasMetadata(std::filesystem::path metadataFilePath)
    : mMetadataFilePath(metadataFilePath)
    , mData(ParseJsonFile(metadataFilePath)) {
}

bool CAtlasMetadata::CheckForChanges(
    const std::vector<CImageFile>& files) const {
    for (const auto& data : mData) {
        if (files.size() != data.entries.size()) {
            return true;
        }
        for (size_t i = 0; i < files.size(); ++i) {
            const auto& file = files[i];
            const auto& entry = data.entries[i];
            if (file.fileSize() != entry.fileSize ||
                file.lastModified() != entry.lastModified) {
                return true;
            }
        }
    }

    return false;
}
