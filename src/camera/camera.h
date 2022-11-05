#pragma once

#include <glm/glm.hpp>
#include <src/utils/scenedata.h>

// A class representing a virtual camera.
class Camera {
public:
    Camera(SceneCameraData cameraData, float imgWidth, float imgHeight);
    Camera() = default;

    void constructBasisVectors();

    // Returns the view matrix for the current camera settings.
    glm::mat4 getViewMatrix() const;
    glm::mat4 getCameraMatrix() const; // transformation from camera space to world space (inverse of view matrix)

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height/width angle of the camera in RADIANS.
    float getHeightAngle() const;
    float getWidthAngle() const;

    float getFocalLength() const;
    float getAperture() const;

    // returns the position of the camera in world space
    glm::vec3 getPos() const;
    
private:
    float m_aspectRatio;
    float m_heightAngle;
    float m_widthAngle;
    float m_focalLength;
    float m_aperature;
    glm::vec4 m_pos;
    glm::vec4 m_look;
    glm::vec4 m_up;
    // basis vectors
    glm::vec3 m_u;
    glm::vec3 m_v;
    glm::vec3 m_w;
};
