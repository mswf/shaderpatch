#pragma once

#include "com_ptr.hpp"
#include "project.hpp"

#include <atomic>

#include <glm/glm.hpp>
#include <imgui.h>

#include <Windows.h>
#include <d3d11_1.h>

namespace sp::editor {

class Editor {
public:
   Editor() = delete;
   explicit Editor(HWND window);

   Editor(const Editor&) = delete;
   Editor& operator=(const Editor&) = delete;
   Editor(Editor&&) = delete;
   Editor& operator=(Editor&&) = delete;

   void update();

   void window_size_changed(glm::ivec2 size) noexcept;

   void activated() noexcept;
   void deactivated() noexcept;

   void suspended() noexcept;
   void resumed() noexcept;

   static constexpr auto window_title = L"Shader Patch Editor";
   static constexpr auto window_class_name = L"Shader Patch Editor";

private:
   void create_device();
   void create_swap_chain();
   void create_resources();
   void init_imgui_impl();

   void handle_device_lost();

   void open_project();

   const HWND _window;
   glm::ivec2 _window_size;

   const std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> _imgui_context;

   Com_ptr<ID3D11Device1> _device;
   Com_ptr<ID3D11DeviceContext1> _device_context;
   Com_ptr<IDXGISwapChain1> _swap_chain;

   Com_ptr<ID3D11RenderTargetView> _bacK_buffer;

   bool _project_unopened = true;
   Project _project;

   static constexpr auto _backbuffer_format = DXGI_FORMAT_B8G8R8A8_UNORM;
   inline static std::atomic_bool _editor_created = false;
};

}
