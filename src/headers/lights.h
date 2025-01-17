#ifndef LIGHTS_H
#define LIGHTS_H

#define LIGHT_TYPE_POINT 1

struct LightData {
    int used;
    int enabled;
    int type;
    float position[3];
    float diffuse[3];
    float specular[3];
    float ambient[3];
    float specPower;
    float intensity;
    float attenuation[3];
};

void LightData_Alloc(struct LightData **ptr, unsigned int count);
void LightData_Dealloc(struct LightData **ptr);

#endif
