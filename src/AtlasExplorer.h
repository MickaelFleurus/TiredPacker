#include <filesystem>

class CAtlasExplorer {
public:
  CAtlasExplorer(std::filesystem::path resourceFolder);

  void Explore();

private:
  const std::filesystem::path mResourceFolder;
};
