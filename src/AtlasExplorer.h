#include <filesystem>

class CAtlasExplorer {
public:
    CAtlasExplorer(const std::filesystem::path& resourceFolder);

    int Explore(const std::filesystem::path& outputFolder);

private:
    const std::filesystem::path& mResourceFolder;
};
