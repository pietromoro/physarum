#include "memory.h"
#include "windows.h"

void* ph_alloc(u32 size) {
  return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}

void* ph_realloc(void* address, u32 newSize) {
  return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, address, newSize);
}

b32 ph_free(void* address) {
  return HeapFree(GetProcessHeap(), 0, address);
}