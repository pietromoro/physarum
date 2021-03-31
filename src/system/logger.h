#pragma once
#include "defines.h"

typedef enum log_level {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
} log_level;

void log_output_log(log_level level, const u8* file, i32 line, const u8* fmt, ...);

void logger_initialize(void);
void logger_destroy(void);

#define PH_FATAL(...) log_output_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define PH_ERROR(...) log_output_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define PH_WARN(...) log_output_log(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define PH_INFO(...) log_output_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define PH_DEBUG(...) log_output_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define PH_TRACE(...) log_output_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define PH_LOG(...) log_output_log(LOG_LEVEL_INFO, "", 0, __VA_ARGS__)