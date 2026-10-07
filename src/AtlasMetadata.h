#pragma once

#include <filesystem>
#include <vector>

#include "ImageFile.h"

class CAtlasMetadata {
public:
    struct SEntry {
        uint width;
        uint height;
        uint x;
        uint y;
        uint64_t fileSize;
        std::string lastModified;
    };
    struct SData {
        std::vector<SEntry> entries;
    };

    CAtlasMetadata(std::filesystem::path metadataFilePath);

    bool CheckForChanges(const std::vector<CImageFile>& files) const;

private:
    std::vector<SData> mData;
    const std::filesystem::path mMetadataFilePath;
};
