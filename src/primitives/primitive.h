#pragma once
#include "src/ray/ray.h"
#include <glm/glm.hpp>
#include <tuple>
#include "src/utils/sceneparser.h"
#include "src/texture/texture.h"
#include <numbers>

using namespace glm;
enum class Plane {
    XY,
    XZ,
    YZ
};


class Primitive {
public:
    Primitive(RenderShapeData shapeData, std::map<std::string, Texture>& textureDictionary);
    Primitive() = default;

    virtual float getIntersectionT(Ray objSpaceRay) const = 0; // get t in r(t)= p + td
    virtual vec3 getObjSpaceNormal(vec3 objSpacePoint) = 0; // non-normalized normal
    vec3 applyCTM(vec3 objSpacePoint, bool isVector) const;
    vec3 applyInverseCTM(vec3 worldSpacePoint, bool isVector) const;
    vec3 getWorldSpaceNormal(vec3 objSpacePoint); // normalized normal
    SceneMaterial getMaterial() const;
    
    SceneColor getTexture(vec3 surfacePointWorldSpace);


protected:
    std::tuple<float, float, float> getXYZComponents(vec3 vector) const;
    float solveQuadratic(float A, float B, float C) const;
    std::tuple<float, float> solveQuadraticBothSolutions(float A, float B, float C) const;
    float intersectPlane(vec3 rayDir, vec3 rayPos, Plane plane, float planeOffset) const;
    float infinity = std::numeric_limits<float>::infinity();
    float getSmallest(std::vector<float>& list) const;

    // texture mapping
    virtual vec2 XYZtoUV(vec3 XYZ) = 0; // takes in OBJECT space XYZ coords
    float getCircleU(float a, float b);

private:
    mat4 m_CTM; 
    mat4 m_inverseCTM;
    mat3 m_objToWorldNormalTransformation;
    ScenePrimitive m_primitiveInfo;
    // SceneFileMap m_textureMap; // already stores loaded texture img
    Texture m_texture;
    SceneFileMap m_textureInfo; // needed for primitive-dependent repeatU, repeatV values
};


class Sphere : public Primitive {
public:
    Sphere() = default;
    Sphere(RenderShapeData commonShapeData, std::map<std::string, Texture>& textureDictionary, float radius):
        m_radius(radius),
        Primitive(commonShapeData, textureDictionary) // call base constructor with data
    {};

    float getIntersectionT(Ray objSpaceRay) const;
    vec3 getObjSpaceNormal(vec3 objSpacePoint);
    vec2 XYZtoUV(vec3 XYZ);
private:
    float m_radius;
};


class Cone : public Primitive {
public:
    Cone() = default;
    Cone(RenderShapeData commonShapeData, std::map<std::string, Texture>& textureDictionary, float baseRadius, float height):
        m_baseRadius(baseRadius),
        m_height(height),
        Primitive(commonShapeData, textureDictionary) // call base constructor with data
    {};

    float getIntersectionT(Ray objSpaceRay) const;
    vec3 getObjSpaceNormal(vec3 objSpacePoint);
    vec2 XYZtoUV(vec3 XYZ);
private:
    float m_baseRadius;
    float m_height;
};


class Cube : public Primitive {
public:
    Cube() = default;
    Cube(RenderShapeData commonShapeData, std::map<std::string, Texture>& textureDictionary, float sideLength):
        m_sideLength(sideLength),
        Primitive(commonShapeData, textureDictionary) // call base constructor with data
    {};

    float getIntersectionT(Ray objSpaceRay) const;
    vec3 getObjSpaceNormal(vec3 objSpacePoint);
    vec2 XYZtoUV(vec3 XYZ);
private:
    float m_sideLength;
};


class Cylinder : public Primitive {
public:
    Cylinder() = default;
    Cylinder(RenderShapeData commonShapeData, std::map<std::string, Texture>& textureDictionary, float height, float radius):
        m_height(height),
        m_radius(radius),
        Primitive(commonShapeData, textureDictionary) // call base constructor with data
    {};

    float getIntersectionT(Ray objSpaceRay) const;
    vec3 getObjSpaceNormal(vec3 objSpacePoint);
    vec2 XYZtoUV(vec3 XYZ);
    
private:
    float m_height = 1;
    float m_radius = 0.5;
};
