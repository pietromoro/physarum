#pragma once
#include "defines.h"

// TODO: Forward declare WINDOWPLACEMENT
#include "windows.h"

typedef struct window {
  u64* handle;
  HGLRC opengl;
  
  u32 width, height;
  f32 aspect_ratio;
  
  b32 is_visible;
  b32 close_requested;
  b32 size_changed;
  WINDOWPLACEMENT placement;
} window;

window* window_create(const u8* title, u32 width, u32 height);
void window_destroy(window* window);
void window_poll_events(window* window);

void window_show(window* window);
void window_fullscreen_toggle(window* window);