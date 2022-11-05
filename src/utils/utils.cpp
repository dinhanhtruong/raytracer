#include "rgba.h"

RGBA toRGBA(const glm::vec4 &illumination) {
    // clamp
    uint8_t r = 255.f*std::clamp(illumination[0], 0.f, 1.f);
    uint8_t g = 255.f*std::clamp(illumination[1], 0.f, 1.f);
    uint8_t b = 255.f*std::clamp(illumination[2], 0.f, 1.f);

    return RGBA{r, g, b};
};

SceneColor RGBAtoSceneColor(RGBA color) {
    return glm::vec4(
        (float) color.r / 255.f,
        (float) color.g / 255.f,
        (float) color.b / 255.f,
        (float) color.a / 255.f // 255 alpha default
    );
}

