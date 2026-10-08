
#include <filesystem>

#include <argparse/argparse.hpp>
#include <fpng.h>

#include "AtlasExplorer.h"

int main(int argc, char* argv[]) {

    fpng::fpng_init();
    std::filesystem::path path, outputFolder;
    argparse::ArgumentParser program("TiredPacker");
    program.add_argument("rootFolder")
        .help("Path to the resource folder")
        .required()
        .store_into(path);
    program.add_argument("-o")
        .help("Output folder for the generated atlas")
        .required()
        .store_into(outputFolder);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return -1;
    }
    std::filesystem::create_directories(outputFolder);

    CAtlasExplorer explorer(path);
    return explorer.Explore(outputFolder);
}
