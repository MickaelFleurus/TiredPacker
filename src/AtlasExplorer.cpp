#include "AtlasExplorer.h"

#include <format>
#include <iostream>

#include <fpng.h>

#include "AtlasUtils.h"

namespace {
constexpr std::string_view kAtlasMetadataFileName{"{}_metadata.json"};
} // namespace

CAtlasExplorer::CAtlasExplorer(const std::filesystem::path& resourceFolder)
    : mResourceFolder(resourceFolder) {
    fpng::fpng_init();
}

int CAtlasExplorer::Explore(const std::filesystem::path& outputFolder) {
    int failedGenerations = 0;
    for (auto const& dir_entry :
         std::filesystem::directory_iterator{mResourceFolder}) {
        if (!dir_entry.is_directory()) {
            continue;
        }

        std::filesystem::path outMetadataPath{
            outputFolder / std::format(kAtlasMetadataFileName,
                                       dir_entry.path().filename().string())};
        auto files = Utility::GatherFiles(dir_entry.path());

        if (std::filesystem::exists(outMetadataPath)) {
            try {
                if (Utility::AreMetadataValid(outMetadataPath, files)) {
                    continue;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error checking metadata validity: " << e.what()
                          << std::endl;
                std::filesystem::remove(outMetadataPath);
            }
        }
        try {
            Utility::Generate(outputFolder,
                              dir_entry.path().filename().string(), files);
        } catch (const std::exception& e) {
            std::cerr << "Error generating atlas: " << e.what() << std::endl;
            failedGenerations++;
        }
    }
    return failedGenerations;
}
