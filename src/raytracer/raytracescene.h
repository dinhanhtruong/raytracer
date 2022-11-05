#pragma once

#include "utils/rgba.h"
#include "utils/sceneparser.h"
#include <memory>
#include <glm/glm.hpp>
#include "primitives/primitive.h"
#include "camera/camera.h"

class Camera;
class Light;

// A class representing a scene to be ray-traced

class RayTraceScene
{
public:
    RayTraceScene(int width, int height, const RenderData &metaData);

    // The getter of the width of the scene
    const int& width() const;

    // The getter of the height of the scene
    const int& height() const;

    // The getter of the global data of the scene
    const SceneGlobalData& getGlobalData() const;

    // The getter of the shared pointer to the camera instance of the scene
    const Camera& getCamera() const;

    const RenderData& getRenderData() const;

    std::vector<std::shared_ptr<Primitive>> getPrimitives() const;
    std::vector<Light> getLights() const;

    

private:
    int m_imgWidth;
    int m_imgHeight;
    RenderData m_renderData; // contains lights, shapes, global data and cam data
    Camera m_camera;
    std::vector<std::shared_ptr<Primitive>> m_primitiveList{};
    std::vector<Light> m_lights{};

    std::map<std::string, Texture> m_textureDictionary{};

};
