#include "primitive.h"


/**
 * @brief Sphere::getIntersectionT returns the smallest non-negative t value at the intersection point. If no intersection exists, returns infinity.
 * @param objSpaceRay
 * @return
 */
float Sphere::getIntersectionT(Ray objSpaceRay) const {
    vec3 rayDir = objSpaceRay.getDir();
    vec3 rayPos = objSpaceRay.getOrigin();

    // get the x/y/z components of the ray's position and dir
    auto [dx, dy, dz] = getXYZComponents(rayDir);
    auto [px, py, pz] = getXYZComponents(rayPos);

    // get coefficients of hand-calculated quadratic equation
    float A = pow(dx, 2) + pow(dy, 2) + pow(dz, 2); // t^2 coeff
    float B = 2*px*dx + 2*py*dy + 2*pz*dz; // t coeff
    float C = pow(px, 2) + pow(py, 2) + pow(pz, 2) - pow(m_radius, 2); // constant term

    // solve for t using quadratic formula
    return solveQuadratic(A, B, C);
}

vec3 Sphere::getObjSpaceNormal(vec3 objSpacePoint) {
    auto [px, py, pz] = getXYZComponents(objSpacePoint);
    // grad f = <f_x, f_y, f_z>
    return vec3(2*px, 2*py, 2*pz);
}



// takes in OBJECT space XYZ coords
vec2 Sphere::XYZtoUV(vec3 XYZ) {
    // u = % of perimeter swept starting from the x-axis, v = linear fn of latitude
    float V = asinf(XYZ[1]/m_radius)/M_PI  + 0.5; // numerator = latitude in range [-pi/2, pi/2]
    float U = (V == 0 | V == 1) ? 0.5 : getCircleU(XYZ[0], XYZ[2]); // U can be anything at the north/south poles

    return vec2(U,V);
}
