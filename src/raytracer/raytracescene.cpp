#include <stdexcept>
#include "raytracescene.h"
#include "src/utils/scenedata.h"
#include "lights/light.h"
#include "texture/texture.h"


RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData) {
    m_camera = Camera(metaData.cameraData, width, height);
    m_imgHeight = height;
    m_imgWidth = width;
    m_renderData = metaData;
    // populate lights
    for (SceneLightData lightData : metaData.lights) {
        m_lights.push_back(Light(lightData));
    }
    
    // build unique textures
    for (auto& shapeData : metaData.shapes) {
        SceneFileMap textureMap = shapeData.primitive.material.textureMap;
        std::string currTextureFilename = textureMap.filename;
        // check if texture with the current filename has already been loaded
        if (textureMap.isUsed && m_textureDictionary.find(currTextureFilename) == m_textureDictionary.end()) {
            // load texture if the file hasn't been loaded before
            m_textureDictionary[currTextureFilename] = Texture(currTextureFilename);
        }
        
    }

    // populate primitiveList
    for (auto& shapeData : metaData.shapes) {
        switch (shapeData.primitive.type) {
            case PrimitiveType::PRIMITIVE_SPHERE:
                m_primitiveList.push_back(std::make_shared<Sphere>(shapeData, m_textureDictionary, 0.5));
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                m_primitiveList.push_back(std::make_shared<Cone>(shapeData, m_textureDictionary, 0.5, 1));
                break;
            case PrimitiveType::PRIMITIVE_CUBE:
                m_primitiveList.push_back(std::make_shared<Cube>(shapeData, m_textureDictionary, 1));
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                m_primitiveList.push_back(std::make_shared<Cylinder>(shapeData, m_textureDictionary, 1, 0.5));
                break;
        }
    }
}



const int& RayTraceScene::width() const {
    return m_imgWidth;
}

const int& RayTraceScene::height() const {
    return m_imgHeight;
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    return m_renderData.globalData;
}

const Camera& RayTraceScene::getCamera() const {
    return m_camera;
}

const RenderData& RayTraceScene::getRenderData() const {
    return m_renderData;
}

std::vector<std::shared_ptr<Primitive>> RayTraceScene::getPrimitives() const {
    return m_primitiveList;
}
std::vector<Light> RayTraceScene::getLights() const {
    return m_lights;
}
