#include <string.h>

#include "components/base_component.h"
#include "components/component_factory.h"

#include "components/camera_component.h"
#include "components/model_renderer_component.h"

#define TYPE_NAME_MAX_SIZE 64

void componentFactoryCreateComponentFromTypeName(const char *typeName, struct BaseComponent **componentPtr) {
    if (typeName == NULL || componentPtr == NULL || (*componentPtr) != NULL) return;

    struct BaseComponent *newComponent = NULL;

    if (strncmp(COMPONENT_TYPE_NAME_CAMERA, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        allocCameraComponent((struct CameraComponent **) &newComponent);
    } else if (strncmp(COMPONENT_TYPE_NAME_MODEL_RENDERER, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        allocModelRendererComponent((struct ModelRendererComponent **) &newComponent);
    }

    (*componentPtr) = newComponent;
}
