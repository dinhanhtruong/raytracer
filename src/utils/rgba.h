#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <cstdint>
#include "scenedata.h"

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;
};

RGBA toRGBA(const glm::vec4 &illumination);
SceneColor RGBAtoSceneColor(RGBA color);
