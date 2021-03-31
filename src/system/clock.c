#include "clock.h"
#include "memory.h"
#include "logger.h"
#include "windows.h"

inline const f64 get_absolute_time(clock* clock);

clock* clock_create() {
  clock* result = ph_alloc(sizeof(clock));
  
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  result->frequency = 1.0 / (f64)frequency.QuadPart;
  
  return result;
}

void clock_update(clock* clock) {
  if (clock->start != 0) {
    clock->elapsed = get_absolute_time(clock) - clock->start;
  }
}

void clock_start(clock* clock) {
  clock->start = get_absolute_time(clock);
  clock->elapsed = 0;
}

void clock_stop(clock* clock) {
  clock->start = 0;
}

inline const f64 get_absolute_time(clock* clock) {
  LARGE_INTEGER nowTime;
  QueryPerformanceCounter(&nowTime);
  return (f64)nowTime.QuadPart * clock->frequency;
}