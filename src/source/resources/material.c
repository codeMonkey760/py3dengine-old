#include <stdlib.h>

#include "util.h"
#include "custom_string.h"
#include "resources/material.h"

void allocMaterial(struct Material **materialPtr) {
    if (materialPtr == NULL || (*materialPtr) != NULL) return;

    struct Material *newMaterial = calloc(1, sizeof(struct Material));
    if (newMaterial == NULL) return;

    newMaterial->_shader = NULL;
    Vec3Identity(newMaterial->diffuseColor);
    Vec3Identity(newMaterial->ambientColor);
    Vec3Identity(newMaterial->specularColor);
    Vec3Identity(newMaterial->emissiveColor);
    newMaterial->specularPower = 0.0f;
    newMaterial->_name = NULL;

    (*materialPtr) = newMaterial;
    newMaterial = NULL;
}

void deleteMaterial(struct Material **materialPtr) {
    if (materialPtr == NULL || (*materialPtr) == NULL) return;

    struct Material *material = (*materialPtr);

    deleteString(&material->_name);

    free(material);
    material = NULL;
    (*materialPtr) = NULL;
}

struct String *getMaterialName(struct Material *material) {
    if (material == NULL) return NULL;

    return material->_name;
}

void setMaterialName(struct Material *material, const char *newName) {
    if (material == NULL) return;

    if (newName == NULL) {
        deleteString(&material->_name);
        return;
    }

    if (material->_name == NULL) {
        allocString(&material->_name, newName);
    } else {
        setChars(material->_name, newName);
    }
}

void setMaterialShader(struct Material *material, struct Shader *newShader) {
    if (material == NULL) return;

    material->_shader = newShader;
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
