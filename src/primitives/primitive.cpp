#include "primitive.h"

using namespace std;

/**
 * @brief Primitive::Primitive Base primitive class constructor. Unpacks relevant transformation and texture information to be used in member methods.
 * @param shapeData shape-specific RenderShapeData object obtained from the scene parser
 * @param textureDictionary a reference to the already populated mapping from filenames to Textures.
 */
Primitive::Primitive(RenderShapeData shapeData, std::map<std::string, Texture>& textureDictionary) {
    // construct relevant transformation matrices once
    m_CTM = shapeData.ctm;
    m_inverseCTM = inverse(m_CTM);
    // construct object-to-world normal transformation using top left 3x3 submatrix of CTM
    vec3 col1 = m_CTM[0];
    vec3 col2 = m_CTM[1];
    vec3 col3 = m_CTM[2];
    mat3 CTM33 = mat3(col1, col2, col3);
    m_objToWorldNormalTransformation = transpose(inverse(CTM33));
    m_primitiveInfo = shapeData.primitive;
    
    // assign reference to loaded texture to this primitive
    m_texture = textureDictionary[m_primitiveInfo.material.textureMap.filename];
    m_textureInfo = m_primitiveInfo.material.textureMap;
}

/**
 * @brief Primitive::applyCTM Computes Tx where T is the cumulative transformation matrix of this primitive which defines
 *        its position and rotation in world space and x is the input object-space point.
 * @param objSpacePoint an object-space 3D point to be transformed into world space via this Primitive's CTM.
 * @param isVector boolean: if true, then treat objSpacePoint as a vector when converting to homogeneous coordinates, else treat as a point.
 * @return Tx, a vector resulting from applying the CTM to the query object-space point
 */
vec3 Primitive::applyCTM(vec3 objSpacePoint, bool isVector) const {
    // homogeneous coords
    vec4 worldSpacePoint = m_CTM * vec4(objSpacePoint, isVector ? 0 : 1);
    return vec3(worldSpacePoint); 
}

/**
 * @brief Primitive::applyInverseCTM same as applyCTM but uses the inverse CTM instead
 */
vec3 Primitive::applyInverseCTM(vec3 worldSpacePoint, bool isVector) const {
    vec4 objSpacePoint = m_inverseCTM * vec4(worldSpacePoint, isVector ? 0 : 1);
    return vec3(objSpacePoint); 
}

/**
 * @brief Primitive::getWorldSpaceNormal computes the WORLD space normal of this primitive at the given OBJECT space point.
 */
vec3 Primitive::getWorldSpaceNormal(vec3 objSpacePoint) {
    vec3 objSpaceNormal = getObjSpaceNormal(objSpacePoint);
    return normalize(m_objToWorldNormalTransformation * objSpaceNormal);
}



/**
 * @brief Primitive::getXYZComponents helper to get XYZ components of a ray's position and dir
 * @param vector a 3D vector to be unpacked
 * @return a tuple containing three floats; one for each of the x/y/z coordinates stored in the input vector.
 */
std::tuple<float, float, float> Primitive::getXYZComponents(vec3 vector) const {
    return std::make_tuple(vector[0], vector[1], vector[2]);
}


/**
 * @brief Primitive::solveQuadratic Single-solution quadratic solver (returns smallest NON-NEGATIVE solution if one exists, and infinity otherwise). EXCLUDES negative solutions.
 * @param A coefficient of squared term in quadratic
 * @param B coeff of linear term
 * @param C constant term
 * @return  smallest NON-NEGATIVE solution if one exists, and infinity otherwise
 */
float Primitive::solveQuadratic(float A, float B, float C) const {
    float discriminant = pow(B, 2) - 4*A*C;
    if (discriminant >= 0) {
        // return smallest NON-NEGATIVE solution
        float t1 = (-B - sqrt(discriminant))/(2*A);
        float t2 = (-B + sqrt(discriminant))/(2*A);
        if (t1 > 0 && t1 < t2) {
            return t1;
        } else if (t2 > 0 && t2 < t1) {
            return t2;
        }
    }
    return infinity;
}

/**
 * @brief Primitive::solveQuadraticBothSolutions same as solveQuadratic but returns both solutions, even if they are infinity (i.e. do not exist).
 * @return tuple containing the two solutions (infinity if none or one exists) to the quadratic equation.
 */
std::tuple<float, float> Primitive::solveQuadraticBothSolutions(float A, float B, float C) const {
    float discriminant = pow(B, 2) - 4*A*C;
    if (discriminant >= 0) {
        // return both solutions
        float t1 = (-B - sqrt(discriminant))/(2*A);
        float t2 = (-B + sqrt(discriminant))/(2*A);
        return std::make_tuple(t1, t2);
    }
    return std::make_tuple(infinity, infinity);
}


/**
 * @brief Primitive::intersectPlane gives the smallest non-negative t value of the intersection between the specified axis-aligned plane and a world space ray.
 * @param rayDir
 * @param rayPos
 * @param plane one of XY, XZ, or YZ from the Plane enum
 * @param planeOffset e.g. for the YZ plane given by x=0.5, the offset is 0.5.
 * @return smallest NON-NEGATIVE t, if any, else infinity
 */
float Primitive::intersectPlane(vec3 rayDir, vec3 rayPos, Plane plane, float planeOffset) const {
    auto [dx, dy, dz] = getXYZComponents(rayDir);
    auto [px, py, pz] = getXYZComponents(rayPos);
    float intersectionT;
    switch(plane) {
        case Plane::XY:
            // z = p_z + td_z = planeOffset
            intersectionT = (planeOffset - pz) / dz;
            break;
        case Plane::XZ:
            // y = p_y + td_y = planeOffset
            intersectionT = (planeOffset - py) / dy;
            break;
        case Plane::YZ:
            // x = p_x + td_x = planeOffset
            intersectionT = (planeOffset - px) / dx;
            break;
    }
    return intersectionT > 0 ? intersectionT : infinity;
}

/**
 * @brief Primitive::getSmallest helper to return the smallest element in the given list, or infinity if the list is empty
 */
float Primitive::getSmallest(std::vector<float>& list) const {
    return list.empty() ? infinity  :  *std::min_element(list.begin(), list.end());
}

SceneMaterial Primitive::getMaterial() const {
    return m_primitiveInfo.material;
}

// texture mapping
/**
 * @brief Primitive::getTexture retrieves the texture image color corresponding to the given point on the surface of the primitive.
 * @param surfacePointObjSpace point on the surface of the Primitive in object space
 * @return texture color corresponding to the given surface point in [0,1] float format.
 */
SceneColor Primitive::getTexture(vec3 surfacePointObjSpace) {
    if (!m_textureInfo.isUsed) {
        // black if no texture is used for this primitive
        return vec4(0,0,0,1);
    }
    // clip UV to [0,1] to avoid float precision issues (e.g. when at cube edges)
    vec2 UV = glm::clamp( XYZtoUV(surfacePointObjSpace), 0.f, 1.f);

    return RGBAtoSceneColor(m_texture.getTextureColorAtUV(UV, m_textureInfo.repeatU, m_textureInfo.repeatV));
}

/**
 * @brief Primitive::getCircleU computes the U coordinate for a given point on a circle as the percentage of the perimeter swept starting from the horizontal axis.
 * @param a cooordinate on the horizontal axis of a point on the circle
 * @param b cooordinate on the DOWNWARD vertical axis of a point on the circle
 * @return
 */
float Primitive::getCircleU(float a, float b) {
    // (a,b) is a point on the circle where a is the horizontal component and b the vertical (in the downward direction if viewed from above)
    float theta = atan2(b, a); // in [-pi, pi]
    return (theta < 0) ?
        (-theta/(2*std::numbers::pi)) :  // in [0, 0.5]
        (1 - theta/(2*std::numbers::pi)); // in [0.5, 1]
}
