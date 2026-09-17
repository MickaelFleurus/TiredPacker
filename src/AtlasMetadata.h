#pragma once

#include <string>
#include <vector>

namespace tired {
struct AtlasSprite {
    std::string name;
    int x = 0, y = 0, width = 0, height = 0;
    int sourceWidth = 0, sourceHeight = 0;
    int offsetX = 0, offsetY = 0;
};
struct AtlasMetadata {
    int width = 0, height = 0;
    std::vector<AtlasSprite> sprites;
};
}
