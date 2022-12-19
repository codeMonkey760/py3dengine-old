#ifndef PY3DENGINE_SHADER_H
#define PY3DENGINE_SHADER_H

extern void enableShader();
extern void disableShader();

extern void initShader();
extern void deleteShader();

extern void setDiffuseColor(float newDiffuseColor[3]);
void setCameraPosition(float newCameraPos[3]);
void setWMtx(float newWMtx[16]);
void setWITMtx(float newWITMtx[16]);
extern void setWVPMtx(float newWVPMtx[16]);

#endif
