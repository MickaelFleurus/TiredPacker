
// Go through the folders
// Check if there was a change in the files
// Regenerate the sprite sheet if there was a change

#include <argparse/argparse.hpp>
#include <filesystem>

#include "AtlasExplorer.h"

int main(int argc, char *argv[]) {
  std::filesystem::path path;
  argparse::ArgumentParser program("TiredPacker");
  program.add_argument("rootFolder")
      .help("Path to the resource folder")
      .required()
      .store_into(path);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  CAtlasExplorer explorer(path);
  explorer.Explore();
  return 0;
}
