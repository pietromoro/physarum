#include "opengl.h"
#include "window.h"
#include "logger.h"

#include "windows.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef BOOL (WINAPI* wgl_swap_interval_ext)(i32);
typedef HGLRC (WINAPI* wgl_create_context_attribs_arb)(HDC, HGLRC, const i32*);

static wgl_swap_interval_ext wglSwapInterval;
static wgl_create_context_attribs_arb wglCreateContextAttribsARB;

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
PFNGLMEMORYBARRIERPROC glMemoryBarrier;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLBUFFERSUBDATAPROC glBufferSubData;

PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

static void opengl_load_extensions(void) {
  wglCreateContextAttribsARB = (wgl_create_context_attribs_arb)wglGetProcAddress("wglCreateContextAttribsARB");
  wglSwapInterval = (wgl_swap_interval_ext)wglGetProcAddress("wglSwapIntervalEXT");
  
  glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
  glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
  glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)wglGetProcAddress("glBindImageTexture");
  glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)wglGetProcAddress("glMemoryBarrier");
  glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)wglGetProcAddress("glGetIntegeri_v");
  glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute");
  glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase");
  glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
  glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
  glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
}

b32 opengl_initialize(window* window) {
  if (!window->handle) {
    PH_FATAL("Window is not initialized.");
    return 0;
  }
  
  HDC dc = GetDC((HWND)window->handle);
  
  PIXELFORMATDESCRIPTOR desiredFormat;
  memset(&desiredFormat, 0, sizeof(PIXELFORMATDESCRIPTOR));
  desiredFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  desiredFormat.nVersion = 1;
  desiredFormat.iPixelType = PFD_TYPE_RGBA;
  desiredFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  desiredFormat.cColorBits = 24;
  desiredFormat.cAlphaBits = 8;
  desiredFormat.iLayerType = PFD_MAIN_PLANE;
  
  i32 suggestedFormatIndex = ChoosePixelFormat(dc, &desiredFormat);
  if (suggestedFormatIndex == 0) {
    PH_FATAL("Could not choose a pixel format.");
    ReleaseDC((HWND)window->handle, dc);
    return 0;
  }
  
  PIXELFORMATDESCRIPTOR suggestedFormat;
  DescribePixelFormat(dc, suggestedFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &suggestedFormat);
  if (!SetPixelFormat(dc, suggestedFormatIndex, &suggestedFormat)) {
    PH_FATAL("Could not set a pixel format.");
    ReleaseDC((HWND)window->handle, dc);
    return 0;
  }
  
  HGLRC tmpContext = wglCreateContext(dc);
  if (!wglMakeCurrent(dc, tmpContext)) {
    PH_FATAL("OpenGL initialization failed");
    ReleaseDC((HWND)window->handle, dc);
    return 0;
  }
  
  i32 attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 4,
    WGL_CONTEXT_FLAGS_ARB, 0,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  
  opengl_load_extensions();
  
  if (!wglCreateContextAttribsARB) {
    PH_FATAL("Couldn't get necessary wgl function.");
    ReleaseDC((HWND)window->handle, dc);
    wglDeleteContext(tmpContext);
    return 0;
  }
  
  HGLRC openGLRC = wglCreateContextAttribsARB(dc, tmpContext, attributes);
  wglDeleteContext(tmpContext);
  if (!openGLRC) {
    PH_FATAL("Couldn't create a modern OpenGL context.");
    ReleaseDC((HWND)window->handle, dc);
    return 0;
  }
  
  if (!wglMakeCurrent(dc, openGLRC)) {
    PH_FATAL("Modern OpenGL initialization failed");
    wglDeleteContext(openGLRC);
    ReleaseDC((HWND)window->handle, dc);
    return 0;
  }
  
  window->opengl = openGLRC;
  
  if (wglSwapInterval)
    wglSwapInterval(1);
  
  ReleaseDC((HWND)window->handle, dc);
  return 1;
}

void opengl_destroy(window* window) {
  HDC dc = GetDC((HWND)window->handle);
  wglMakeCurrent(dc, 0);
  wglDeleteContext((HGLRC)window->opengl);
  ReleaseDC((HWND)window->handle, dc);
  window->opengl = 0;
}

void opengl_context(window* window) {
  if (window->handle && window->opengl) {
    HDC dc = GetDC((HWND)window->handle);
    wglMakeCurrent(dc, window->opengl);
    ReleaseDC((HWND)window->handle, dc);
  }
}

void opengl_swap_buffers(window* window) {
  HDC dc = GetDC((HWND)window->handle);
  SwapBuffers(dc);
  ReleaseDC((HWND)window->handle, dc);
}