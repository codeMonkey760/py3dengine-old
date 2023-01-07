#include <string.h>

#include "components/base_component.h"
#include "components/component_factory.h"

#include "components/camera_component.h"
#include "components/model_renderer_component.h"
#include "components/rotation_component.h"
#include "components/python_component.h"

#define TYPE_NAME_MAX_SIZE 64

void componentFactoryCreateComponentFromJson(const char *typeName, struct BaseComponent **componentPtr) {
    if (typeName == NULL || componentPtr == NULL || (*componentPtr) != NULL) return;

    struct BaseComponent *newComponent = NULL;

    if (strncmp(COMPONENT_TYPE_NAME_CAMERA, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        allocCameraComponent((struct CameraComponent **) &newComponent);
    } else if (strncmp(COMPONENT_TYPE_NAME_MODEL_RENDERER, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        allocModelRendererComponent((struct ModelRendererComponent **) &newComponent);
    } else if (strncmp(COMPONENT_TYPE_NAME_ROTATION, typeName, TYPE_NAME_MAX_SIZE) == 0) {
        allocRotationComponent((struct RotationComponent **) &newComponent);
    } else {
        allocPythonComponent((struct PythonComponent **) &newComponent);
    }

    (*componentPtr) = newComponent;
}
