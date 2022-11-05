#include "sceneparser.h"
#include "scenefilereader.h"
#include "glm/gtx/transform.hpp"

#include <chrono>
#include <memory>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readXML();
    if (!success) {
        return false;
    }

    // populate renderData with global data, lights, and camera data
    renderData.globalData = fileReader.getGlobalData();
    renderData.lights = fileReader.getLights();
    renderData.cameraData = fileReader.getCameraData();

    // populate renderData's list of primitives and their transforms by traversing scene graph
    renderData.shapes.clear();
    SceneNode* root = fileReader.getRootNode();
    glm::mat4 identity = glm::mat4(1.0f);
    calculateCTM(identity, root, renderData.shapes);

    return true;
}

// depth-first tree traversal: populates the RenderShapeData list in-place
void SceneParser::calculateCTM(glm::mat4& parentCTM, SceneNode* currNode, std::vector<RenderShapeData>& renderShapesList) {
    // general case: the CTM of currNode is the product of its parent's CTM and currNode's local tranformation(s)
    glm::mat4 currCTM = parentCTM; // store copy of parent CTM
    glm::mat4 currTransformMatrix = glm::mat4(); // will be initialized during loop
    // multiply currCTM by the local transformation matrices stored at this in left-to-right order (so that transformations are applied right-to-left on child)
    for (auto& currTransformation : currNode->transformations) {
        // construct the current local transformation matrix depending on its type
        switch (currTransformation->type) {
            case TransformationType::TRANSFORMATION_SCALE:
                currTransformMatrix = glm::scale(currTransformation->scale);
            break;
            case TransformationType::TRANSFORMATION_ROTATE:
                currTransformMatrix = glm::rotate(currTransformation->angle, currTransformation->rotate);
            break;
            case TransformationType::TRANSFORMATION_TRANSLATE:
                currTransformMatrix = glm::translate(currTransformation->translate);
            break;
            case TransformationType::TRANSFORMATION_MATRIX:
                currTransformMatrix = currTransformation->matrix;
            break;
        }
        currCTM *= currTransformMatrix;
    }

    // if currNode contains at least one primitive, then add the primitive(s) and the current CTM to the render data list
    for (auto& primitive : currNode->primitives) {
        RenderShapeData shapeData{*primitive, currCTM};
        renderShapesList.push_back(shapeData);
    }

    // recurse on children (if any):
    for (auto& childNode : currNode->children) {
        calculateCTM(currCTM, childNode, renderShapesList);
    }


}
