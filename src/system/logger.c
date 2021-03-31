#include "logger.h"
#include "memory.h"
#include "windows.h"

#include <stdarg.h>
#include <stdio.h>

static const u8* level_strs[] = {
  "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

static const u8* level_cols[] = {
  "\x1b[31m", "\x1b[31m", "\x1b[33m", "\x1b[32m", "\x1b[36m", "\x1b[94m"
};

void logger_initialize(void) {
  ASSERT(AllocConsole());
  SetConsoleCtrlHandler(0, 1);
  HWND console = GetConsoleWindow();
  HMENU menu = GetSystemMenu(console, 0);
  DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
}

void logger_destroy(void) {
  ASSERT(FreeConsole());
}

void log_output_log(log_level level, const u8* file, i32 line, const u8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  u8 outMessage[3200];
  i32 outMessageLength = _vsnprintf(outMessage, sizeof(outMessage) - 1, fmt, args);
  va_end(args);
  
  // 7(col) + 6(lvl) + 7(reset) + 7(col) + file.length + 7(reset) + 2(:) + 4(line) + 2 (\0\n)
#if 0 // TODO: Figure out why colors aren't working
  u64 totalLength = outMessageLength + strlen(file) + 44;
  u8* out = (u8*)ph_alloc(sizeof(u8) * totalLength);
  sprintf_s(out, totalLength, "%s%-5s\x1b[0m \x1b[90m%s:%04d:\x1b[0m %s\r", level_cols[level], level_strs[level], file, line, outMessage);
#else
  u64 totalLength = outMessageLength + strlen(file) + 16;
  u8* out = (u8*)ph_alloc(sizeof(u8) * totalLength);
  sprintf_s(out, totalLength, "%-5s %s: %04d: %s", level_strs[level], file, line, outMessage);
#endif
  
  out[totalLength - 2] = '\n';
  out[totalLength - 1] = 0;
  
  //OutputDebugStringA(out);
  DWORD numberWritten = 0;
  HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (level < 2) stdHandle = GetStdHandle(STD_ERROR_HANDLE);
  
#if 0
  DWORD dwMode = 0;
  GetConsoleMode(stdHandle, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
  SetConsoleMode(stdHandle, dwMode);
#endif
  
  WriteConsole(stdHandle, out, (DWORD)(totalLength - 1), &numberWritten, 0);
  
  ph_free(out);
}