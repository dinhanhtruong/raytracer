#include "raytracer.h"
#include "raytracescene.h"
#include "utils/rgba.h"


RayTracer::RayTracer(Config config) :
    m_config(config)
{}

/**
 * @brief RayTracer::render populates the imageData pixel array by shooting a ray through each pixel on the view plane and (recursively) determining each ray's color.
 * @param imageData pointer to an RGBA array containing the colors of the canvas
 * @param scene reference to a RayTraceScene object which contains information about the scene's camera, primitives, and lights.
 */
void RayTracer::render(RGBA *imageData, const RayTraceScene &scene) {
    Camera camera = scene.getCamera();
    m_primitives = scene.getPrimitives();
    m_globalData = scene.getGlobalData();

    // iterate over pixel samples (at pixel centers)
    for (int row = 0; row < scene.height(); row++) {
        for (int col = 0; col < scene.width(); col++) {
            // get coords of curr pixel on view plane in camera space (uvk), pick k=depth=1
            float k = 1.f;
            vec3 uvk = getViewPlaneCoords(row, col, k, scene);
            // get ray direction in camera space
            vec3 rayDirCamSpace = uvk; // eye = <0,0,0> in cam space

            // convert to world space direction using the camera matrix (cam space -> world space)
            vec3 rayDirWorldSpace = camera.getCameraMatrix() * glm::vec4(rayDirCamSpace, 0);
            // construct ray in WORLD space
            Ray ray(rayDirWorldSpace, camera.getPos()); // cam pos is already in world space

            // trace ray to get final pixel color, update image data
            imageData[col + row*scene.width()] = traceRay(ray, m_primitives, scene.getLights(), 0); // start w/ 0 recursion depth
        }
    }
}


/**
 * @brief RayTracer::getViewPlaneCoords returns the coordinate in camera space of an input pixel on the view plane
 * @param row index into the imaginary view plane pixel grid where (0,0) is the top-left pixel
 * @param col
 * @param k depth/distance along the look vector to the view plane
 * @return continuous coordinate in camera space of an input pixel on the view plane
 */
vec3 RayTracer::getViewPlaneCoords(int row, int col, float k, const RayTraceScene &scene) {
    // assume (row,col)=(0,0) is at the top left of the view plane
    Camera camera = scene.getCamera();
    float viewplaneWidth = 2*k*tan(camera.getWidthAngle()/2); // scale factor to be applied to unit viewplane
    float viewplaneHeight = 2*k*tan(camera.getHeightAngle()/2);
    // get xy coords on unit viewplane (centered about the look vec)
    float xNormalized = (col + 0.5)/scene.width() - 0.5;
    float yNormalized = (scene.height() - row - 0.5)/scene.height() - 0.5;
    
    return vec3(xNormalized * viewplaneWidth, yNormalized * viewplaneHeight, -k);
}

/**
 * @brief RayTracer::traceRay traces the given world space ray through the scene and computes the final lighting for the ray (black if ray does not intersect any geometry)
 * @param worldSpaceRay a Ray defined in world space via its origin position and direction
 * @param scene
 * @param currRecursionDepth the current depth in the recursion tree. Recursive rays are not generated if the maximum depth is reached.
 * @return RGBA color corresponding to this ray
 */
RGBA RayTracer::traceRay(Ray &worldSpaceRay, 
                        std::vector<std::shared_ptr<Primitive>> &primitives, 
                        const std::vector<Light> &lights,
                        int currRecursionDepth) {
                            
    // keep track of the intersected primitive (if any) and the object space intersection for normal calculation
    int intersectedPrimitiveIdx;
    vec3 objSpaceIntersection;
    // iterate over all primitives and check for intersections
    for (int i = 0; i < primitives.size(); i++) {
        std::shared_ptr<Primitive> currPrimitive = primitives[i];
        // construct obj space ray from world space ray
        Ray objSpaceRay = Ray(
            currPrimitive->applyInverseCTM(worldSpaceRay.getDir(), true), // direction is a vector
            currPrimitive->applyInverseCTM(worldSpaceRay.getOrigin(), false) // ray origin is a point
        );
        // use primitive's implicit formula to find the nearest intersection (t) in obj space (if any). This t is the same for world space.
        float currT = currPrimitive->getIntersectionT(objSpaceRay);
        
        // store obj space intersection only if valid
        if (currT > 0 && currT < worldSpaceRay.getIntersectionT()) {
            objSpaceIntersection = objSpaceRay.getPos(currT);
            intersectedPrimitiveIdx = i;
            // store current value of t in world space ray bc currPrimitive is the closest yet
            worldSpaceRay.setIntersectionT(currT);
        }
    }
           
    // compute lighting if the given ray is not a shadow ray AND ray-obj intersection exists (i.e. if 0 < t < infinity)
    if (currRecursionDepth != -1 && worldSpaceRay.getIntersectionT() > 0 && worldSpaceRay.getIntersectionT() < std::numeric_limits<float>::infinity()) {
        // compute WORLD space normal and intersection point
        vec3 worldNormal = primitives[intersectedPrimitiveIdx]->getWorldSpaceNormal(objSpaceIntersection); // already normalized
        vec3 worldIntersection = worldSpaceRay.getIntersectionPoint();
        vec3 dirToCamera = -worldSpaceRay.getDir(); // original ray dir is from camera to intersection point

        // compute lighting
        return phong(
            worldIntersection, 
            worldNormal, 
            dirToCamera, 
            primitives[intersectedPrimitiveIdx]->getMaterial(), 
            primitives[intersectedPrimitiveIdx]->getTexture(objSpaceIntersection),
            lights,
            m_globalData,
            currRecursionDepth // used to recursively call traceRay when lighting
        );
    }
    // if no intersection, return black
    return RGBA{0,0,0};
}

/**
 * @brief RayTracer::phong (recursively) computes the RGBA color at the given point from the given view direction using the phong lighting equation.
 *          Handles shadows by ignoring the contribution of occlued light sources
 * @param intersectionPosition position at which the ray first intersects scene geometry. In world space.
 * @param normal world-space normal of the intersected object at the intersection point
 * @param directionToCamera vector determining the direction from the intersection position to the viewer
 * @param material SceneMaterial containing object-specific color and lighting coefficients
 * @param textureColor color retrieved from the texture image at the intersection point
 * @param lights vector of Lights in the scene
 * @param globalData SceneGlobalData containing scene coefficients needed in the Phong lighting equation
 * @param currRecursionDepth
 * @return RGBA color corresponding to the given ray
 */
RGBA RayTracer::phong(
           glm::vec3  intersectionPosition,
           glm::vec3  normal,
           glm::vec3  directionToCamera,
           SceneMaterial material,
           SceneColor textureColor, // color of texture img at the intersection position
           const std::vector<Light>& lights,
           SceneGlobalData globalData,
           int currRecursionDepth) {
    // normalizing directions
    normal            = glm::normalize(normal);
    directionToCamera = glm::normalize(directionToCamera);


    // output illumination (ignore opacity)
    glm::vec4 totalIllumination(0, 0, 0, 1);

    // add the ambient term
    totalIllumination += globalData.ka * material.cAmbient;

    for (Light light : lights) {
        SceneLightData lightData = light.getLightData();
        glm::vec3 directionToLight = light.getDirToLight(intersectionPosition);
        float distToLight = (light.getType() == LightType::LIGHT_DIRECTIONAL) ?  std::numeric_limits<float>::infinity() : glm::length(vec3(lightData.pos) - intersectionPosition);
        // compute attenuation factor
        float f_att = (light.getType() == LightType::LIGHT_DIRECTIONAL) ? 1 : light.attenuationFn(distToLight); // no attenuation for directional lights (only for spot/point lights)

        // shoot shadow ray to determine visibility of primary intersection point
        vec3 shadowRayOrigin = intersectionPosition + 0.001f*directionToLight; // add epsilon to avoid self-shadowing
        Ray shadowRayWorldSpace(directionToLight, shadowRayOrigin);
        // shoot shadow ray at special recursion depth=-1 so that no further recursive rays are traced. Ignore color output of traceRay.
        traceRay(shadowRayWorldSpace, m_primitives, lights, -1); // stores intersection (if any) in the passed shadow ray
        if (shadowRayWorldSpace.getIntersectionT() < distToLight) {
            // shadow ray to light is occluded bc intersection exists BEFORE ray reaches light: ignore this light's contribution
            continue;
        }

        // only add diffuse/specular light if normal faces toward the camera (i.e. angle < 90 deg)
        float NdotL = glm::dot(normal, directionToLight);
        if (NdotL > 0 ) {
            SceneColor lightColor = light.getColor(intersectionPosition);
            
            // add the diffuse term (linearly interpolated material color and texture color)
            SceneColor diffuseColor = material.blend * textureColor  +  (1-material.blend)*(globalData.kd * material.cDiffuse);
            totalIllumination += f_att * (lightColor * diffuseColor) * NdotL;

            // add the specular term I*k*O*(R*V)^n
            glm::vec3 reflectedLightDirection = glm::reflect(-directionToLight, normal);
            float RdotV = std::max(0.f, glm::dot(reflectedLightDirection, directionToCamera));
            totalIllumination += f_att * (lightColor * globalData.ks * material.cSpecular) * std::pow(RdotV, material.shininess);
        }
    }

    // add reflection
    if (currRecursionDepth < m_maxRecursionDepth) {
        glm::vec3 reflectedViewDirection = glm::reflect(-directionToCamera, normal);
        // glm::vec3 reflectedViewDirection = 2.f*glm::dot(directionToCamera, normal)*normal - directionToCamera;
        
        // shoot reflection across normal
        Ray reflectionRay(reflectedViewDirection, intersectionPosition + 0.0001f*reflectedViewDirection); // add epsilon to avoid self-reflections
        SceneColor reflectionColor = RGBAtoSceneColor(traceRay(reflectionRay, m_primitives, lights, currRecursionDepth + 1));
        
        // add contribution of reflection to the final intensity of this ray's pixel
        totalIllumination += globalData.ks * material.cReflective * reflectionColor;
    }

    RGBA returnValue = toRGBA(totalIllumination);
    return returnValue;
}

