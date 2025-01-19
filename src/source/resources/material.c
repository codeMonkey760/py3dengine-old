#include <stdlib.h>

#include "util.h"
#include "custom_string.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "logger.h"

#define RESOURCE_TYPE_MATERIAL 1

static void delete(struct BaseResource **resourcePtr) {
    if (resourcePtr == NULL) return;

    if (!isResourceTypeMaterial((*resourcePtr))) return;

    deleteMaterial((struct Material **) resourcePtr);
}

bool isResourceTypeMaterial(struct BaseResource *resource) {
    if (resource == NULL) return false;

    return resource->_type = RESOURCE_TYPE_MATERIAL && stringEqualsCStr(resource->_typeName, RESOURCE_TYPE_NAME_MATERIAL);
}

void allocMaterial(struct Material **materialPtr) {
    if (materialPtr == NULL || (*materialPtr) != NULL) return;

    struct Material *newMaterial = calloc(1, sizeof(struct Material));
    if (newMaterial == NULL) return;

    struct BaseResource *base = (struct BaseResource *) newMaterial;
    initializeBaseResource(base);
    base->_type = RESOURCE_TYPE_MATERIAL;
    allocString(&base->_typeName, RESOURCE_TYPE_NAME_MATERIAL);
    base->delete = delete;
    base = NULL;

    newMaterial->_shader = NULL;
    Vec3Identity(newMaterial->diffuseColor);
    Vec3Identity(newMaterial->ambientColor);
    Vec3Identity(newMaterial->specularColor);
    Vec3Identity(newMaterial->emissiveColor);
    newMaterial->specularPower = 0.0f;

    (*materialPtr) = newMaterial;
    newMaterial = NULL;
}

void deleteMaterial(struct Material **materialPtr) {
    if (materialPtr == NULL || (*materialPtr) == NULL) return;

    struct Material *material = (*materialPtr);
    material->_diffuseMap = NULL;

    finalizeBaseResource((struct BaseResource *) material);

    free(material);
    material = NULL;
    (*materialPtr) = NULL;
}

void setMaterialShader(struct Material *material, struct Shader *newShader) {
    if (material == NULL) return;

    material->_shader = newShader;
}

void setMaterialDiffuseMap(struct Material *material, struct Texture *newDiffuseMap) {
    if (material == NULL || newDiffuseMap == NULL) return;

    if (material->_diffuseMap != NULL) {
        warning_log("%s", "[Material]: Overwriting previous diffuse map. It may be leaked");
    }

    material->_diffuseMap = newDiffuseMap;
}

const float *getMaterialDiffuseColor(struct Material *material) {
    if (material == NULL) return NULL;

    return material->diffuseColor;
}

void setMaterialDiffuseColorRGB(struct Material *material, float r, float g, float b) {
    if (material == NULL) return;

    material->diffuseColor[0] = r;
    material->diffuseColor[1] = g;
    material->diffuseColor[2] = b;
}

void setMaterialDiffuseColor(struct Material *material, const float newDiffuseColor[3]) {
    if (material == NULL || newDiffuseColor == NULL) return;

    Vec3Copy(material->diffuseColor, newDiffuseColor);
}

const float *getMaterialAmbientColor(struct Material *material) {
    if (material == NULL) return NULL;

    return material->ambientColor;
}

void setMaterialAmbientColorRGB(struct Material *material, float r, float g, float b) {
    if (material == NULL) return;

    material->ambientColor[0] = r;
    material->ambientColor[1] = g;
    material->ambientColor[2] = b;
}

void setMaterialAmbientColor(struct Material *material, const float newAmbientColor[3]) {
    if (material == NULL || newAmbientColor == NULL) return;

    Vec3Copy(material->ambientColor, newAmbientColor);
}

const float *getMaterialSpecularColor(struct Material *material) {
    if (material == NULL) return NULL;

    return material->specularColor;
}

void setMaterialSpecularColorRGB(struct Material *material, float r, float g, float b) {
    if (material == NULL) return;

    material->specularColor[0] = r;
    material->specularColor[1] = g;
    material->specularColor[2] = b;
}

void setMaterialSpecularColor(struct Material *material, const float newSpecularColor[3]) {
    if (material == NULL || newSpecularColor == NULL) return;

    Vec3Copy(material->specularColor, newSpecularColor);
}
