#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "ray/ray.h"
#include "utils/scenedata.h"
#include "lights/light.h"
#include "raytracescene.h"

using namespace glm;

class Ray;
class RayTraceScene;

// A class representing a ray-tracer
class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
    };

public:
    RayTracer(Config config);

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    void render(RGBA *imageData, const RayTraceScene &scene);

private:
    const Config m_config;
    std::vector<std::shared_ptr<Primitive>> m_primitives{};
    SceneGlobalData m_globalData;
    int m_maxRecursionDepth = 4;

    // helpers (see raytracer.cpp for documentation)
    vec3 getViewPlaneCoords(int row, int col, float k, const RayTraceScene &scene);
    RGBA traceRay(Ray &worldSpaceRay, std::vector<std::shared_ptr<Primitive>> &primitives, const std::vector<Light> &lights, int currRecursionDepth);
    RGBA phong(glm::vec3  position,
               glm::vec3  normal,
               glm::vec3  directionToCamera,
               SceneMaterial  material,
               SceneColor textureColor, // color of texture img at the intersection position
               const std::vector<Light>& lights,
               SceneGlobalData globalData,
               int currRecursionDepth);
    
};

