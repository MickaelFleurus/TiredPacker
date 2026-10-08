#pragma once
#include <filesystem>
#include <map>

#include "ImageFile.h"

namespace Utility {
bool AreMetadataValid(const std::filesystem::path& metadataFilePath,
                      const std::map<std::string, CImageFile>& files);
void Generate(const std::filesystem::path& outputFolder,
              const std::string& name,
              std::map<std::string, CImageFile>& files);
std::map<std::string, CImageFile>
GatherFiles(const std::filesystem::path& assetsFolder);

}; // namespace Utility
