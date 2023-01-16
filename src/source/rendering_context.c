#include <stdlib.h>

#include "logger.h"
#include "util.h"
#include "rendering_context.h"
#include "game_object.h"
#include "components/camera_component.h"
#include "python/py3dtransform.h"

static void extractVPMatrix(struct GameObject *activeCamera, float dst[16]) {
    if (activeCamera == NULL || dst == NULL) return;


}

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

void initRenderingContext(struct RenderingContext *context, struct GameObject *activeCamera) {
    if (context == NULL || activeCamera == NULL) return;

    struct Py3dTransform *transform = getGameObjectTransform(activeCamera);
    if (transform == NULL) {
        critical_log("%s", "[RenderingContext]: Could not query Transform Component from the active camera game object. It's probably malformed.");
        return;
    }

    struct CameraComponent *cameraComponent = NULL;
    cameraComponent = (struct CameraComponent *) getGameObjectComponentByType(activeCamera, COMPONENT_TYPE_NAME_CAMERA);
    if (cameraComponent == NULL) {
        error_log("%s", "[RenderingContext]: Could not query Camera Component from the active camera game object. It's probably malformed.");
        return;
    }

    Mat4Mult(
        context->vpMtx,
        getTransformViewMtx(getGameObjectTransform(activeCamera)),
        getCameraComponentProjMatrix(cameraComponent)
    );

    Vec3Copy(context->cameraPositionW, transform->position);
}
