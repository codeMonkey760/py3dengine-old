#include <stdlib.h>
#include <string.h>
#include "lights.h"

void LightData_Alloc(struct LightData **ptr, unsigned int count) {
    if (ptr == NULL || *ptr != NULL || count == 0) return;

   *ptr = calloc(count, sizeof(struct LightData));
}

void LightData_Dealloc(struct LightData **ptr) {
    if (ptr == NULL || *ptr == NULL) return;

    free(*ptr);
    *ptr = NULL;
}

void LightData_Init(struct LightData *self) {
    if (self == NULL) return;

    memset(self, 1, sizeof(struct LightData));
}

void LightData_Copy(struct LightData *dst, const struct LightData *src) {
    if (dst == NULL || src == NULL) return;

    memcpy(dst, src, sizeof(struct LightData));
}