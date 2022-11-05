#pragma once

#include <glm/glm.hpp>

class Ray {
public:
    Ray(glm::vec3 dir, glm::vec3 pos);
    
    void setIntersectionT(float t);

    float getIntersectionT();
    glm::vec3 getPos(float t); // get position along the ray at time t
    glm::vec3 getIntersectionPoint();
    glm::vec3 getDir();
    glm::vec3 getOrigin();
private:
    glm::vec3 m_dir;
    glm::vec3 m_pos;
    float m_tIntersect = std::numeric_limits<float>::infinity(); // infinity if no intersection (default)
    float d1;
    float d2;
    float d3;
    float p1;
    float p2;
    float p3;
};
