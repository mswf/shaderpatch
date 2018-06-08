
#include "editor.hpp"
#include "icon_resources.h"

#include <ios>
#include <memory>

#include <imgui.h>

#include <Windows.h>

extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

using namespace sp::editor;

ATOM register_class(HINSTANCE instance);
HWND create_window(HINSTANCE, int);
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg,
                                                 WPARAM wParam, LPARAM lParam);
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
   cls.lpszClassName = Editor::window_class_name;
   cls.hIconSm = LoadIconW(instance, MAKEINTRESOURCE(IDI_EDITOR_SMALL_ICON));

   return RegisterClassExW(&cls);
}

HWND create_window(HINSTANCE instance, int cmd_show)
{
   HWND window = CreateWindowExW(0, Editor::window_class_name, Editor::window_title,
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

   if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
      switch (message) {
      case WM_LBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONDBLCLK:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONDBLCLK:
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
      case WM_MOUSEWHEEL:
      case WM_MOUSEHWHEEL:
      case WM_MOUSEMOVE:
      case WM_SETCURSOR:
         if (ImGui::GetIO().WantCaptureMouse) {
            return ImGui_ImplWin32_WndProcHandler(wnd, message, wparam, lparam);
         }
         break;
      case WM_KEYDOWN:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYUP:
      case WM_CHAR:
         if (ImGui::GetIO().WantCaptureKeyboard) {
            return ImGui_ImplWin32_WndProcHandler(wnd, message, wparam, lparam);
         }
      }
   }

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
   case WM_MOUSEMOVE:
      return ImGui_ImplWin32_WndProcHandler(wnd, message, wparam, lparam);
   default:
      return DefWindowProcW(wnd, message, wparam, lparam);
   }
   return 0;
}
