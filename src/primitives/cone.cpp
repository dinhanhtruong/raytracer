#include "primitive.h"

/**
 * @brief Cone::getIntersectionT computes the smallest non-negative 'time' parameter t at which the object space ray r(t) = p + td
 *          interects this cone. The cone is aligned vertically with the y-axis
 * @param objSpaceRay Ray defined in object space (i.e. with origin p and direction d in object space)
 * @return float t where the intersection of r(t) and the surface occurs closest to the camera
 */
float Cone::getIntersectionT(Ray objSpaceRay) const {
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

    //// 1) Conical top
    // get coefficients of hand-calculated quadratic equation
    float A = pow(dx, 2) + pow(dz, 2) - (1/4.f)*pow(dy, 2); // t^2 coeff
    float B = 2*px*dx + 2*pz*dz - (1/2.f)*py*dy + (1/4.f)*dy; // t coeff
    float C = pow(px, 2) + pow(pz, 2) - (1/4.f)*pow(py, 2) + (1/4.f)*py - (1/16.f); // constant term
    auto [t1, t2] = solveQuadraticBothSolutions(A, B, C);

    // bounds checking
    // solution is valid iff y in [-h/2, h/2], but must check both solutions since cone is reflected about apex
    if (t1 < infinity | t2 < infinity) {
        float yIntersect1 = objSpaceRay.getPos(t1)[1]; // y coord of intersection point
        float yIntersect2 = objSpaceRay.getPos(t2)[1]; 
        if ( yIntersect1 >= -m_height/2 && yIntersect1 <= m_height/2) {
            intersectionTList.push_back(t1);
        } 
        if ( yIntersect2 >= -m_height/2 && yIntersect2 <= m_height/2) {
            intersectionTList.push_back(t2);
        } 
    }
    

    //// 2) flat base
    // get (single) intersection with y=-h/2 plane
    currT = (-0.5 - py) / dy;

    // bounds checking
    // solution valid iff inside disk bounded by baseRadius on xz-plane
    if (currT < infinity) {
        xIntersect = objSpaceRay.getPos(currT)[0];
        zIntersect = objSpaceRay.getPos(currT)[2];
        if (pow(xIntersect, 2) + pow(zIntersect, 2) <= pow(m_baseRadius, 2)) {
            intersectionTList.push_back(currT);
        }
    }
    

    // return smallest t (if no intersection, this is infinity)
    return getSmallest(intersectionTList);
}


/**
 * @brief Cone::getObjSpaceNormal Computes the object-space normal of the given object-space point
 * @param objSpacePoint a point on the cone's surfacesurface in object space
 * @return non-normalized object-space normal.
 */
vec3 Cone::getObjSpaceNormal(vec3 objSpacePoint) {
    auto [px, py, pz] = getXYZComponents(objSpacePoint);
    // constant normal for base
    if (std::abs(py + m_height/2) < 0.0001) { // epsilon to handle float precision
        // normal on flat base is in -y direction
        return vec3(0,-1,0);
    }
    // grad f = <f_x, f_y, f_z>)
    return vec3(2*px, 0.25 - 0.5*py, 2*pz);
}

/**
 * @brief Cone::XYZtoUV Maps a given XYZ point to a unique, normalized UV point in [0,1]^2 via a surface parametrization.
 * @param XYZ an object-space 3D point on the surface of the cone
 * @return 2-dimensional UV coordinates uniquely corresponding to the given XYZ point
 */
vec2 Cone::XYZtoUV(vec3 XYZ) {
    if (std::abs(XYZ[1] + m_height/2) < 0.0001) { // flat base on y=-0.5 plane
        // use shifted coordinates in planar circle as UV
        return vec2(XYZ[0], XYZ[2]) + 0.5f;
    }
    // conical top: u = % of perimeter traveled, v = offset height
    float u = (XYZ[1] == m_height/2) ? 0.5 : getCircleU(XYZ[0], XYZ[2]); // if at apex of cone (i.e. where x=z=0), pick any u
    return vec2(u, XYZ[1] + 0.5f);
}
