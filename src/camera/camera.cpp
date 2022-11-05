#include <stdexcept>
#include "camera.h"
#include <src/utils/scenedata.h>
#include "glm/gtx/transform.hpp"
using namespace glm;

Camera::Camera(SceneCameraData cameraData, float imgWidth, float imgHeight) {
    m_aspectRatio = imgWidth / imgHeight;
    m_heightAngle = cameraData.heightAngle;
    m_widthAngle = m_heightAngle * m_aspectRatio;
    m_focalLength = cameraData.focalLength;
    m_aperature = cameraData.aperture;
    m_look = cameraData.look;
    m_pos = cameraData.pos;
    m_up = cameraData.up;
    constructBasisVectors();
}

/**
 * @brief Camera::constructBasisVectors constructs the orthogonal basis for the camera space (u,v,w) defined by the look and up vectors
 */
void Camera::constructBasisVectors() {
    // get cartesian 3D representation of look and up
    vec3 look(m_look);
    vec3 up(m_up);
    // construct u,v,w basis vecs
    m_w = normalize(-look);
    vec3 vertical = (up - dot(up, m_w)*m_w); // orthogonal to w in up-look plane
    m_v = normalize(vertical); // unit length
    m_u = cross(m_v, m_w);
}

/**
 * @brief Camera::getViewMatrix
 * @return the inverse of the camera matrix which positions and rotates the camera in world space
 */
mat4 Camera::getViewMatrix() const {
    // inverse of camera matrix: rotation is transposed and translation is in the opposite dir
    vec4 col1(m_u[0], m_v[0], m_w[0], 0.f);
    vec4 col2(m_u[0], m_v[0], m_w[0], 0.f);
    vec4 col3(m_u[0], m_v[0], m_w[0], 0.f);
    vec4 inverseTranslation = -m_pos;
    inverseTranslation[3] = 1;
    
    return mat4(col1, col2, col3, inverseTranslation);
}

mat4 Camera::getCameraMatrix() const {
    // convert to homogeneous
    return mat4(vec4(m_u, 0.f), vec4(m_v, 0.f), vec4(m_w, 0.f), m_pos);
}

float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

float Camera::getHeightAngle() const {
    return m_heightAngle;
}

float Camera::getWidthAngle() const {
    return m_widthAngle;
}

float Camera::getFocalLength() const {
    return m_focalLength;
}

float Camera::getAperture() const {
    return m_aperature;
}

glm::vec3 Camera::getPos() const {
    return m_pos;
}
