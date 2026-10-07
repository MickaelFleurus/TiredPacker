#include "AtlasExplorer.h"

#include "AtlasMetadata.h"
#include "GeneratedAtlas.h"

namespace {
constexpr std::string_view atlasMetadataFileName = "atlas_metadata.json";
} // namespace

CAtlasExplorer::CAtlasExplorer(std::filesystem::path resourceFolder)
    : mResourceFolder(resourceFolder) {
}

void CAtlasExplorer::Explore() {

    for (auto const& dir_entry :
         std::filesystem::directory_iterator{mResourceFolder}) {
        if (dir_entry.is_directory()) {
            CGeneratedAtlas atlas(dir_entry.path());
            if (std::filesystem::exists(dir_entry.path() /
                                        atlasMetadataFileName)) {
                // Verify if the file have changed.
                CAtlasMetadata metadata(dir_entry.path() /
                                        atlasMetadataFileName);
                if (!metadata.CheckForChanges(atlas.GetFiles())) {
                    continue;
                }
            }
            atlas.TryGenerate();
        }
    }
}
