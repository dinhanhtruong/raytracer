#include "primitive.h"

/**
 * @brief isInSquare a helper to check whether the given 2D point lies within a square with given side length sitting at the origin
 * @param sideLength square side length
 * @return
 */
bool isInSquare(float x, float y, float sideLength) {
    return std::fabs(x) <= sideLength/2 && std::fabs(y) <= sideLength/2;
}

/**
 * @brief Cube::getIntersectionT computes the smallest non-negative 'time' parameter t at which the object space ray r(t) = p + td
 *          interects this cube. The cube is aligned vertically with the y-axis
 * @param objSpaceRay Ray defined in object space (i.e. with origin p and direction d in object space)
 * @return float t where the intersection of r(t) and the surface occurs closest to the camera
 */
float Cube::getIntersectionT(Ray objSpaceRay) const {
    vec3 rayDir = objSpaceRay.getDir();
    vec3 rayPos = objSpaceRay.getOrigin();

    // temp vars
    float currT;
    std::vector<float> intersectionTList{};
    vec3 intersectionPoint;


    //// 1) intersections w/ faces parallel to XY plane
    // find closest intersection with planes z = 0.5*sideLen, z=-0.5*sideLen
    currT = std::min( 
        intersectPlane(rayDir, rayPos, Plane::XY, 0.5), // non-negative number or infinity
        intersectPlane(rayDir, rayPos, Plane::XY, -0.5)
    );
    intersectionPoint = objSpaceRay.getPos(currT);
    // bounds checking (intersection must be in unit square centered at origin in the curr plane)
    if ( currT < infinity && isInSquare(intersectionPoint[0], objSpaceRay.getPos(currT)[1], 1) ) {
        intersectionTList.push_back(currT);
    }
    
    //// 2) intersections w/ faces parallel to XZ plane
    // find closest intersection with planes y = 0.5*sideLen, y=-0.5*sideLen
    currT = std::min(
        intersectPlane(rayDir, rayPos, Plane::XZ, 0.5),
        intersectPlane(rayDir, rayPos, Plane::XZ, -0.5)
    );
    intersectionPoint = objSpaceRay.getPos(currT);
    // bounds checking
    if ( currT < infinity && isInSquare(intersectionPoint[0], objSpaceRay.getPos(currT)[2], 1) ) {
        intersectionTList.push_back(currT);
    }


    //// 3) intersections w/ faces parallel to YZ plane
    // find closest intersection with planes x = 0.5*sideLen, x=-0.5*sideLen
    currT = std::min(
        intersectPlane(rayDir, rayPos, Plane::YZ, 0.5),
        intersectPlane(rayDir, rayPos, Plane::YZ, -0.5)
    );
    intersectionPoint = objSpaceRay.getPos(currT);
    // bounds checking
    if ( currT < infinity && isInSquare(intersectionPoint[1], objSpaceRay.getPos(currT)[2], 1) ) {
        intersectionTList.push_back(currT);
    }

    // return smallest t (if no intersection, this is infinity)
    return getSmallest(intersectionTList);
}

/**
 * @brief Cube::getObjSpaceNormal Computes the object-space normal of the given object-space point
 * @param objSpacePoint a point on the cube's surfacesurface in object space
 * @return non-normalized object-space normal.
 */
vec3 Cube::getObjSpaceNormal(vec3 objSpacePoint) {
    auto [px, py, pz] = getXYZComponents(objSpacePoint);
    // get normal based on intersected face (with epsilon to handle float precision)
    if (std::fabs(px - 0.5) < 0.001) {
        return vec3(1, 0, 0); // intersection with face at x=0.5
    } else if (std::fabs(px + 0.5) < 0.0001) {
        return vec3(-1, 0, 0); // face at x=-0.5
    } else if (std::fabs(py - 0.5) < 0.0001) {
        return vec3(0, 1, 0);
    } else if (std::fabs(py + 0.5) < 0.0001) {
        return vec3(0, -1, 0);
    } else if (std::fabs(pz - 0.5) < 0.0001) {
        return vec3(0, 0, 1);
    } else {
        return vec3(0, 0, -1);
    }
}

/**
 * @brief Cube::XYZtoUV Maps a given XYZ point to a unique, normalized UV point in [0,1]^2 via a surface parametrization.
 * @param XYZ an object-space 3D point on the surface of the cube
 * @return 2-dimensional UV coordinates uniquely corresponding to the given XYZ point
 */
vec2 Cube::XYZtoUV(vec3 XYZ) {
    if (std::fabs(XYZ[0] - 0.5) < 0.001) {  // intersection with face at x=0.5
        // normalize YZ face to have values in [0,1] from [-sideLen/2, sideLen/2]
        return (vec2(-XYZ[2], XYZ[1]) / m_sideLength) + 1/2.f;
    } else if (std::fabs(XYZ[0] + 0.5) < 0.0001) { // face at x=-0.5
        // horizontally reflected vs x=0.5 face
        return (vec2(XYZ[2], XYZ[1]) / m_sideLength) + 1/2.f;
    } else if (std::fabs(XYZ[1] - 0.5) < 0.0001) { // face at y=0.5
        return (vec2(XYZ[0], -XYZ[2]) / m_sideLength) + 1/2.f;
    } else if (std::fabs(XYZ[1] + 0.5) < 0.0001) { // face at y=-0.5
        return (vec2(XYZ[0], XYZ[2]) / m_sideLength) + 1/2.f;
    } else if (std::fabs(XYZ[2] - 0.5) < 0.0001) { // face at z=0.5
        return (vec2(XYZ[0], XYZ[1]) / m_sideLength) + 1/2.f;
    } else { // face at z=-0.5
        return (vec2(-XYZ[0], XYZ[1]) / m_sideLength) + 1/2.f;
    }
    return vec2(0,0);
}
