#ifndef PY3DENGINE_MATERIAL_H
#define PY3DENGINE_MATERIAL_H

struct Material {
    float diffuseColor[3];  //Kd
    float ambientColor[3];  //Ka
    float specularColor[3]; //Ks
    float emissiveColor[3]; //Ke

    float specularPower;    //Ns
    //float transparency; //d

    struct Shader *_shader;
    struct String *_name;
};

extern void allocMaterial(struct Material **materialPtr);
extern void deleteMaterial(struct Material **materialPtr);

extern struct String *getMaterialName(struct Material *material);
extern void setMaterialName(struct Material *material, const char *newName);
extern void setMaterialShader(struct Material *material, struct Shader *newShader);

extern const float *getMaterialDiffuseColor(struct Material *material);
extern void setMaterialDiffuseColorRGB(struct Material *material, float r, float g, float b);
extern void setMaterialDiffuseColor(struct Material *material, const float newDiffuseColor[3]);

#endif
