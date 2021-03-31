#pragma once
#include "defines.h"

typedef struct clock {
  f64 start;
  f64 elapsed;
  f64 frequency;
} clock;

clock* clock_create();
void clock_update(clock* clock);
void clock_start(clock* clock);
void clock_stop(clock* clock);