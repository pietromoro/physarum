#include "window.h"
#include "opengl.h"
#include "logger.h"
#include "memory.h"

#include "resource.h"
#include "windows.h"

LRESULT CALLBACK _win32_process_messages(HWND hwnd, u32 u_msg, WPARAM w_param, LPARAM l_param);

window* window_create(const u8* title, u32 width, u32 height) {
  window* out_window = ph_alloc(sizeof(window));
  
  out_window->width = width;
  out_window->height = height;
  out_window->aspect_ratio = (f32)width / (f32)height;
  out_window->size_changed = 0;
  
  HINSTANCE instance = GetModuleHandleA(0);
  
  u32 win_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW;
  u32 win_ex_style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  
  WNDCLASSEX windowClass;
  memset(&windowClass, 0, sizeof(windowClass));
  windowClass.cbSize = sizeof(WNDCLASSEX);
  windowClass.style =  win_ex_style;
  windowClass.lpfnWndProc = _win32_process_messages;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursor(0, IDC_ARROW);
  windowClass.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON));
  windowClass.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON));
  windowClass.lpszClassName = title;
  
  RECT border_rectangle = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rectangle, win_style, 0, win_ex_style);
  width += border_rectangle.right - border_rectangle.left;
  height += border_rectangle.bottom - border_rectangle.top;
  
  if (RegisterClassExA(&windowClass)) {
    HWND handle = CreateWindowExA(0, windowClass.lpszClassName, title, win_style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, 0);
    if (handle) {
      SetWindowLongPtrA(handle, GWLP_USERDATA, (LONG_PTR)out_window);
      out_window->handle = (u64*)handle;
      if (opengl_initialize(out_window)) return out_window;
      return 0;
    } else {
      PH_FATAL("Could not create a window.");
      return 0;
    }
  } else {
    PH_FATAL("Could not register a window class.");
    return 0;
  }
  
  return out_window;
}

void window_destroy(window* window) {
  if (window->handle) {
    opengl_destroy(window);
    DestroyWindow((HWND)window->handle);
  }
  window->handle = 0;
  window->width = window->height = 0;
  window->is_visible = 0;
}

void window_poll_events(window* window) {
  MSG message;
  while (PeekMessage(&message, (HWND)window->handle, 0, 0, PM_REMOVE) > 0) {
    switch (message.message) {
      case WM_QUIT: window->close_requested = 1; break;
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        u32 VKCode = (u32)message.wParam;
        b32 wasDown = ((message.lParam & (1 << 30)) != 0);
        b32 isDown = ((message.lParam & (1 << 31)) == 0);
        
        if (wasDown != isDown) {
          switch (VKCode) {
            case VK_RETURN:
            // TODO: toggle fullscreen
            break;
          }
        } break;
        default:
        TranslateMessage(&message);
        DispatchMessageA(&message);
      }
    }
  }
}

void window_show(window* window) {
  if (window->handle && !window->is_visible) {
    b32 should_activate = 1;
    i32 command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    ShowWindow((HWND)window->handle, command_flags);
    window->is_visible = 1;
  }
}

void window_fullscreen_toggle(window* window) {
  DWORD style = GetWindowLong((HWND)window->handle, GWL_STYLE);
  if (style & WS_OVERLAPPEDWINDOW) {
    MONITORINFO monitorInfo;
    memset(&monitorInfo, 0, sizeof(MONITORINFO));
    monitorInfo.cbSize = sizeof(MONITORINFO);
    if (GetWindowPlacement((HWND)window->handle, &window->placement) && GetMonitorInfo(MonitorFromWindow((HWND)window->handle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
      SetWindowLong((HWND)window->handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
      SetWindowPos((HWND)window->handle, HWND_TOP, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  } else {
    u32 resizable_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW;
    SetWindowLong((HWND)window->handle, GWL_STYLE, style | resizable_style);
    SetWindowPlacement((HWND)window->handle, &window->placement);
    SetWindowPos((HWND)window->handle, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

LRESULT CALLBACK _win32_process_messages(HWND hwnd, u32 u_msg, WPARAM w_param, LPARAM l_param) {
  window* p_window = (window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
  
  switch (u_msg) {
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      ASSERT(!"Keyboard input came in through non-dispatch message");
    } break;
    case WM_SIZE: {
      p_window->width = LOWORD(l_param);
      p_window->height = HIWORD(l_param);
      p_window->aspect_ratio = p_window->width / p_window->height;
      p_window->size_changed = 1;
    } return 0;
    case WM_CLOSE:
    case WM_DESTROY:
    p_window->close_requested = 1;
    return 0;
  }
  
  return DefWindowProcA(hwnd, u_msg, w_param, l_param);
}