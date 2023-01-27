#ifndef PY3DENGINE_MATERIAL_H
#define PY3DENGINE_MATERIAL_H

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_MATERIAL "Material"

struct Material {
    struct BaseResource _base;

    float diffuseColor[3];  //Kd
    float ambientColor[3];  //Ka
    float specularColor[3]; //Ks
    float emissiveColor[3]; //Ke

    float specularPower;    //Ns
    //float transparency; //d

    struct Shader *_shader;
    struct Texture *_diffuseMap;
};

extern bool isResourceTypeMaterial(struct BaseResource *resource);
extern void allocMaterial(struct Material **materialPtr);
extern void deleteMaterial(struct Material **materialPtr);

extern void setMaterialShader(struct Material *material, struct Shader *newShader);
extern void setMaterialDiffuseMap(struct Material *material, struct Texture *newDiffuseMap);

extern const float *getMaterialDiffuseColor(struct Material *material);
extern void setMaterialDiffuseColorRGB(struct Material *material, float r, float g, float b);
extern void setMaterialDiffuseColor(struct Material *material, const float newDiffuseColor[3]);

#endif
