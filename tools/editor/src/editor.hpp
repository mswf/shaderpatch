#pragma once

#include "com_ptr.hpp"

#include <glm/glm.hpp>

#include <Windows.h>
#include <d3d11_1.h>

namespace sp::editor {

class Editor {
public:
   Editor() = delete;
   explicit Editor(HWND window);

   Editor(const Editor&) = default;
   Editor& operator=(const Editor&) = default;
   Editor(Editor&&) = default;
   Editor& operator=(Editor&&) = default;

   void update();

   void window_size_changed(glm::ivec2 size) noexcept;

   void activated() noexcept;
   void deactivated() noexcept;

   void suspended() noexcept;
   void resumed() noexcept;

private:
   void create_device();
   void create_swap_chain();
   void create_resources();

   void handle_device_lost();

   const HWND _window;
   glm::ivec2 _window_size;

   Com_ptr<ID3D11Device1> _device;
   Com_ptr<ID3D11DeviceContext1> _device_context;
   Com_ptr<IDXGISwapChain1> _swap_chain;

   Com_ptr<ID3D11RenderTargetView> _bacK_buffer;

   static constexpr auto _backbuffer_format = DXGI_FORMAT_B8G8R8A8_UNORM;
};

}
