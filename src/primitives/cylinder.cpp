#include "primitive.h"

/**
 * @brief Cylinder::getIntersectionT computes the smallest non-negative 'time' parameter t at which the object space ray r(t) = p + td
 *          interects this cylinder. The cube is aligned vertically with the y-axis
 * @param objSpaceRay Ray defined in object space (i.e. with origin p and direction d in object space)
 * @return float t where the intersection of r(t) and the surface occurs closest to the camera
 */
float Cylinder::getIntersectionT(Ray objSpaceRay) const {
    vec3 rayDir = objSpaceRay.getDir();
    vec3 rayPos = objSpaceRay.getOrigin();

    // get the x/y/z components of the ray's position and dir
    auto [dx, dy, dz] = getXYZComponents(rayDir);
    auto [px, py, pz] = getXYZComponents(rayPos);

    // temp vars
    float currT;
    std::vector<float> intersectionTList{};
    float xIntersect;
    float yIntersect;
    float zIntersect;

    //// 1) intersections w/ flat caps parallel to XZ plane
    // find closest intersection with planes y = 0.5*height, z=-0.5*height
    currT = std::min( 
        intersectPlane(rayDir, rayPos, Plane::XZ, 0.5), // non-negative number or infinity
        intersectPlane(rayDir, rayPos, Plane::XZ, -0.5)
    );
    
    // bounds checking (intersection must be within disk of the same radius as the cylinder)
    xIntersect = objSpaceRay.getPos(currT)[0];
    zIntersect = objSpaceRay.getPos(currT)[2];
    if (pow(xIntersect, 2) + pow(zIntersect, 2) <= pow(m_radius, 2)) {
        intersectionTList.push_back(currT);
    }


    //// 2) intersections w/ cylinder body
    // find closest intersection with infinite cylinder x^2 + z^2 = r^2
    // get coefficients of hand-calculated quadratic equation
    float A = pow(dx, 2) + pow(dz, 2); // t^2 coeff
    float B = 2*(dx*px + dz*pz); // t coeff
    float C = pow(px, 2) + pow(pz, 2) - pow(m_radius, 2); // constant term
    currT = solveQuadratic(A, B, C); // non-negative
    if (currT < infinity) {
        // bounds checking: y-coord of intersection must be within [-height/2, height/2]
        yIntersect = objSpaceRay.getPos(currT)[1];
        if (yIntersect >= -m_height/2 && yIntersect <= m_height/2) {
            intersectionTList.push_back(currT);
        }
    }
    
    // return smallest t (if no intersection, this is infinity)
    return getSmallest(intersectionTList);
}


vec3 Cylinder::getObjSpaceNormal(vec3 objSpacePoint) {
    auto [px, py, pz] = getXYZComponents(objSpacePoint);
    // get normal based on intersected face (with epsilon to handle float precision)
    if (std::fabs(py - 0.5) < 0.0001) {
        return vec3(0, 1, 0); // intersection with face at x=0.5
    } else if (std::fabs(py + 0.5) < 0.0001) {
        return vec3(0, -1, 0); // face at x=-0.5
    } else {
        // intersection with cylinder body: normal points outward from origin
        return vec3(2*px, 0, 2*pz);
    }
}

/**
 * @brief Cylinder::XYZtoUV Maps a given XYZ point to a unique, normalized UV point in [0,1]^2 via a surface parametrization.
 * @param XYZ an object-space 3D point on the surface of the cylinder
 * @return 2-dimensional UV coordinates uniquely corresponding to the given XYZ point
 */
vec2 Cylinder::XYZtoUV(vec3 XYZ) {
    if (std::fabs(XYZ[1] - 0.5) < 0.0001) { // intersection with face at y=0.5
        // use shifted coordinates in planar circle as UV
        return vec2(XYZ[0], -XYZ[2]) + 0.5f;
    } else if (std::fabs(XYZ[1] + 0.5) < 0.0001) { // face at y=-0.5
        return vec2(XYZ[0], XYZ[2]) + 0.5f;
    } else { // intersection with cylinder body
        // u = % of perimeter swept, v = shifted height
        return vec2(getCircleU(XYZ[0], XYZ[2]), XYZ[1] + 0.5);
    }
}
