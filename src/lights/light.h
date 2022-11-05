#pragma once

#include "utils/rgba.h"
#include "utils/scenedata.h"
#include <glm/glm.hpp>

using namespace glm;

class Light {
public:
    Light(SceneLightData lightData);

    SceneLightData getLightData();
    vec3 getDirToLight(vec3 currPosition); // override in spotlight or make visibility scale factor (1 or 0 dep on dir)
    SceneColor getColor(vec3 currPosition); // override in spotlight w falloff
    float attenuationFn(float distToLight);
    LightType getType();

private:
    float smoothFallOff(float x);
    SceneLightData m_lightData;
    LightType m_type;
    float c1;
    float c2;
    float c3;
};
