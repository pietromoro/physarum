#pragma once
#include "defines.h"

typedef struct shader_info {
  u32 type;
  u8* source;
} shader_info;

// NOTE: info is a null terminated array
u32 shader_create(shader_info* info);