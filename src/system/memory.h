#pragma once
#include "defines.h"

void* ph_alloc(u32 size);
void* ph_realloc(void* address, u32 newSize);
b32 ph_free(void* address);