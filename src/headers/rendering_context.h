#ifndef PY3DENGINE_RENDERING_CONTEXT_H
#define PY3DENGINE_RENDERING_CONTEXT_H

struct RenderingContext {
    float vpMtx[16];
    float cameraPositionW[3];
};

extern void allocRenderingContext(struct RenderingContext **contextPtr);
extern void deleteRenderingContext(struct RenderingContext **contextPtr);

#endif
