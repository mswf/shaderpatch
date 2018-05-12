
#include "editor.hpp"
#include "icon_resources.h"

#include <ios>
#include <memory>

#include <Windows.h>

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

using namespace sp::editor;

constexpr auto window_title = L"Shader Patch Editor";
constexpr auto window_class_name = L"Shader Patch Editor";

ATOM register_class(HINSTANCE instance);
HWND create_window(HINSTANCE, int);
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);

namespace {
std::unique_ptr<Editor> editor{};
}

int CALLBACK wWinMain(HINSTANCE instance, HINSTANCE,
                      [[maybe_unused]] LPWSTR cmd_line, int cmd_show)
{
   std::ios_base::sync_with_stdio(false);

   if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) return 1;

   auto atom = register_class(instance);
   auto window = create_window(instance, cmd_show);

   if (!atom || !window) return 1;

   editor = std::make_unique<Editor>(window);

   MSG msg{};

   while (WM_QUIT != msg.message) {
      if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
         TranslateMessage(&msg);
         DispatchMessageW(&msg);
      }
      else {
         editor->update();
      }
   }

   return static_cast<int>(msg.wParam);
}

ATOM register_class(HINSTANCE instance)
{
   WNDCLASSEXW cls;

   cls.cbSize = sizeof(WNDCLASSEX);
   cls.style = CS_HREDRAW | CS_VREDRAW;
   cls.lpfnWndProc = &wnd_proc;
   cls.cbClsExtra = 0;
   cls.cbWndExtra = 0;
   cls.hInstance = instance;
   cls.hIcon = LoadIconW(instance, MAKEINTRESOURCE(IDI_EDITOR_ICON));
   cls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
   cls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
   cls.lpszMenuName = nullptr;
   cls.lpszClassName = window_class_name;
   cls.hIconSm = LoadIconW(instance, MAKEINTRESOURCE(IDI_EDITOR_SMALL_ICON));

   return RegisterClassExW(&cls);
}

HWND create_window(HINSTANCE instance, int cmd_show)
{
   HWND window = CreateWindowExW(0, window_class_name, window_title,
                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT,
                                 0, nullptr, nullptr, instance, nullptr);

   ShowWindow(window, cmd_show);
   UpdateWindow(window);

   return window;
}

LRESULT CALLBACK wnd_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
   if (!editor) return DefWindowProcW(wnd, message, wparam, lparam);

   static bool in_sizemove = false;
   static bool in_suspend = false;
   static bool minimized = false;

   switch (message) {
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   case WM_SIZE:
      if (wparam == SIZE_MINIMIZED) {
         if (!std::exchange(minimized, true)) {
            if (!std::exchange(in_suspend, true)) editor->suspended();
         }
      }
      else if (minimized) {
         minimized = false;
         if (std::exchange(in_suspend, false)) editor->resumed();
      }
      else if (!in_sizemove) {
         editor->window_size_changed({LOWORD(lparam), HIWORD(lparam)});
      }
      break;
   case WM_ENTERSIZEMOVE:
      in_sizemove = true;
      break;
   case WM_EXITSIZEMOVE:
      in_sizemove = false;

      RECT rect;
      GetClientRect(wnd, &rect);

      editor->window_size_changed({rect.right - rect.left, rect.bottom - rect.top});
      break;
   case WM_GETMINMAXINFO: {
      auto info = reinterpret_cast<MINMAXINFO*>(lparam);
      info->ptMinTrackSize.x = 800;
      info->ptMinTrackSize.y = 600;
   } break;

   case WM_ACTIVATEAPP:
      if (wparam) {
         editor->activated();
      }
      else {
         editor->deactivated();
      }
      break;
   default:
      return DefWindowProcW(wnd, message, wparam, lparam);
   }
   return 0;
}
