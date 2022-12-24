#include <stdlib.h>

#include "util.h"
#include "custom_string.h"
#include "material.h"

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

const char * getMaterialName(struct Material *material) {
    if (material == NULL) return NULL;

    return getChars(material->_name);
}

void setMaterialName(struct Material *material, const char *newName) {
    if (material == NULL || newName == NULL) return;

    setChars(material->_name, newName);
}

void setMaterialShader(struct Material *material, struct Shader *newShader) {
    if (material == NULL) return;

    // TODO: is NULL a valid value here? It is after initialization
    material->_shader = newShader;
}