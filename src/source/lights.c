#include <stdlib.h>
#include "lights.h"

void LightData_Alloc(struct LightData **ptr, unsigned int count) {
    if (ptr == NULL || (*ptr) != NULL || count == 0) return;

   (*ptr) = calloc(count, sizeof(struct LightData));
}

void LightData_Dealloc(struct LightData **ptr) {
    if (ptr == NULL || (*ptr) == NULL) return;


}