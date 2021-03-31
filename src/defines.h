#pragma once

// NOTE: Types
#include <stdint.h>
#include <stddef.h>
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i32 b32;

typedef float  f32;
typedef double f64;

typedef size_t memory_index;

// NOTE: Assertions
#ifdef PH_INTERNAL
#if _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() __asm { int 3 }
#endif

#define ASSERT(expr) { \
if (expr) {} else { \
DEBUG_BREAK(); \
} \
}


#define ASSERT_MSG(expr, message) { \
if (expr) {} else { \
DEBUG_BREAK(); \
} \
}
#else
#define ASSERT(expr)
#define ASSERT_MSG(expr, message)
#endif

// Utilities
#define CLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max : value
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define MINIMUM(a, b) ((a < b) ? (a) : (b))
#define MAXIMUM(a, b) ((a > b) ? (a) : (b))

#define DECLARE_TYPE(name) typedef struct name name