#include "ray.h"

Ray::Ray(glm::vec3 dir, glm::vec3 pos) {
    m_dir = dir;
    m_pos = pos;
    d1 = dir[0];
    d2 = dir[1];
    d3 = dir[2];
    p1 = pos[0];
    p2 = pos[1];
    p3 = pos[2];
}

void Ray::setIntersectionT(float t) {
    m_tIntersect = t;
}

float Ray::getIntersectionT() {
    return m_tIntersect;
}

glm::vec3 Ray::getPos(float t) {
    // r(t) = p + td
    return m_pos + t*m_dir;
}

glm::vec3 Ray::getIntersectionPoint() {
    return getPos(m_tIntersect);
}

glm::vec3 Ray::getDir() { 
    return m_dir; 
}

glm::vec3 Ray::getOrigin() {
    return m_pos;
}

