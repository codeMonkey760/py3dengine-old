#ifndef LIGHTS_H
#define LIGHTS_H

#define LIGHT_TYPE_UNKNOWN 0
#define LIGHT_TYPE_POINT 1

struct LightData {
    // TODO: find a way to remove the OpenGL stuff
    GLint used;
    GLint enabled;
    GLint type;
    float position[3];
    float diffuse[3];
    float specular[3];
    float ambient[3];
    float intensity;
    float attenuation[3];
};

extern void LightData_Alloc(struct LightData **ptr, unsigned int count);
extern void LightData_Dealloc(struct LightData **ptr);
extern void LightData_Init(struct LightData *self);
extern void LightData_Copy(struct LightData *dst, const struct LightData *src);

#endif
