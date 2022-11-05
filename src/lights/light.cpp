#include "light.h"

Light::Light(SceneLightData lightData): m_lightData(lightData) {
    m_type = lightData.type;
    // get attentuation constants
    c1 = m_lightData.function[0];
    c2 = m_lightData.function[1];
    c3 = m_lightData.function[2];
}

SceneLightData Light::getLightData() { 
    return m_lightData; 
}

/**
 * @brief Light::getDirToLight
 * @param currPosition query position in world space
 * @return the normalized direction to the light from the given position in world space
 */
vec3 Light::getDirToLight(vec3 currPosition) {
    // handle spotlight/direcitonal/point
    switch (m_type) {
        case LightType::LIGHT_POINT:
        case LightType::LIGHT_SPOT:
            return normalize(vec3(m_lightData.pos) - currPosition);
            break;
        case LightType::LIGHT_DIRECTIONAL:
            // direction to the light is the same regardless of position
            return normalize(-m_lightData.dir);
            break;
        default:
            return normalize(-m_lightData.dir);
    }
}


/**
 * @brief Light::smoothFallOff An easing function for smoothly interpolating between the inner and outer cone angle of a spotlight.
 *          Only works when the current Light is of type Spotlight
 * @param x angle, in radians, relative to the spotlight direction
 * @return 0 if x=inner, 1 if x=outer, smooth in between. 0 if the current light is not a spotlight.
 */
float Light::smoothFallOff(float x) {
    if (m_type != LightType::LIGHT_SPOT) {
        return 0;
    }

    float totalConeAngle = m_lightData.angle;
    float innerConeAngle = totalConeAngle - m_lightData.penumbra;
    float scaledX = (x - innerConeAngle) / (totalConeAngle - innerConeAngle);
    return -2*pow(scaledX, 3) + 3*pow(scaledX, 2);
}

/**
 * @brief Light::getColor computes the color of this light, with special attention to spotlights. For spotlights, the color varies from
 *          its full intensity to 0 between the inner and outer cones. For other light types, this is a position-independent constant color.
 * @param currPosition world space XYZ position from which the light is viewed
 * @return color of this Light in float form.
 */
SceneColor Light::getColor(vec3 currPosition) {
    // handle spotlight separately
    if (m_type == LightType::LIGHT_SPOT) {
        // color depends on direction to the light from the query position
        // get absolute angle between curr direction and spotlight dir (radians, [-pi/2,pi/2])
        vec3 spotlightDir = normalize(vec3(m_lightData.dir));
        vec3 lightToIntersection = currPosition - vec3(m_lightData.pos); // alr normalized
        float distToLight = length(lightToIntersection);

        float theta = std::acos( dot(lightToIntersection, spotlightDir) / distToLight ); // acos(component parallel to light dir / component along diagonal)
        
        if (theta > m_lightData.angle) {
            // ray direction toward light is entirely outside of the light cone
            return vec4(0,0,0,1); // no color
        } else if (theta >= m_lightData.angle - m_lightData.penumbra) {
            // in outer cone: light intensity drops off with distance from the light direction
            return vec4( (1 - smoothFallOff(theta)) * vec3(m_lightData.color), 1);
        }
        // in inner cone: full intensity
    }
    return m_lightData.color;
}

/**
 * @brief Light::attenuationFn computes a scaling factor for the light intensity as a function of distance to the light using pre-defined coefficients
 * @param distToLight
 * @return float in [0,1] that is roughly inversely proportioante to the distance to the light
 */
float Light::attenuationFn(float distToLight) {
    return std::min(1.f, (1/(c1 + distToLight*c2 + pow(distToLight, 2.f)*c3)) );
}

LightType Light::getType() {
    return m_type;
}
