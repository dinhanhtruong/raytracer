#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "utils/rgba.h"

class Texture {
public:
    Texture() = default;
    Texture(std::string filename);
    std::string getFilename();
    RGBA getTextureColorAtUV(glm::vec2 UV, int repeatU, int repeatV);
    std::tuple<int, int> UVtoImgCoord(glm::vec2 UV, int repeatU, int repeatV);
private:
    std::vector<RGBA> m_imgData; // texture img
    int m_width = 0;
    int m_height = 0;
    std::string m_filename;
};
