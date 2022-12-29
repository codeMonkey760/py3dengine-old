#include <stdlib.h>

#include "util.h"
#include "rendering_context.h"

void allocRenderingContext(struct RenderingContext **contextPtr) {
    if (contextPtr == NULL || (*contextPtr) != NULL) return;

    struct RenderingContext *context = calloc(1, sizeof(struct RenderingContext));
    if (context == NULL) return;

    Mat4Identity(context->vpMtx);
    Vec3Identity(context->cameraPositionW);

    (*contextPtr) = context;
    context = NULL;
}

void deleteRenderingContext(struct RenderingContext **contextPtr) {
    if (contextPtr == NULL || (*contextPtr) == NULL) return;

    free( (*contextPtr) );
    (*contextPtr) = NULL;
}