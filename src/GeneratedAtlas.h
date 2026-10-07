#pragma once
#include <filesystem>
#include <vector>

#include "ImageFile.h"

// Get the folder path, check if the atlas metadata exists.
// If it does, check if the files have changed since the last generation.
// If they have, regenerate the atlas and update the metadata.
// If the metadata doesn't exist, generate the atlas and create the metadata.
// If there is any subfolders, trigger an error
class CGeneratedAtlas {
public:
    CGeneratedAtlas(std::filesystem::path atlasFolder);
    ~CGeneratedAtlas() = default;

    void TryGenerate();
    const std::vector<CImageFile>& GetFiles() const;

private:
    const std::filesystem::path mAtlasFolder;
    std::vector<CImageFile> mFiles;
};
