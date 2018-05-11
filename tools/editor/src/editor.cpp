
#include "editor.hpp"
#include "debug_state.hpp"
#include "synced_io.hpp"
#include "throw_if_failed.hpp"

namespace sp::editor {

Editor::Editor(HWND window) : _window{window}
{
   RECT rect{};
   GetClientRect(window, &rect);

   _window_size = {rect.right - rect.left, rect.bottom - rect.top};

   create_device();
   create_swap_chain();
}

void Editor::tick()
{
   _swap_chain->Present(0, 0);
}

void Editor::window_size_changed(glm::ivec2 size) noexcept
{
   _window_size = size;
   _swap_chain->ResizeBuffers(2, _window_size.x, _window_size.y, _backbuffer_format, 0);
}

void Editor::activated() noexcept {}

void Editor::deactivated() noexcept {}

void Editor::suspended() noexcept {}

void Editor::resumed() noexcept {}

void Editor::create_device()
{
   UINT creation_flags = 0;

   if constexpr (debug_build) creation_flags |= D3D11_CREATE_DEVICE_DEBUG;

   static const D3D_FEATURE_LEVEL feature_levels[] = {
      D3D_FEATURE_LEVEL_9_3,
   };

   Com_ptr<ID3D11Device> device;
   Com_ptr<ID3D11DeviceContext> context;
   throw_if_failed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                     creation_flags, feature_levels,
                                     _countof(feature_levels), D3D11_SDK_VERSION,
                                     device.clear_and_assign(), nullptr,
                                     context.clear_and_assign()));

   if constexpr (debug_build) {
      Com_ptr<ID3D11Debug> d3d_debug;

      if (SUCCEEDED(device.as(d3d_debug))) {
         Com_ptr<ID3D11InfoQueue> d3d_info_queue;
         if (SUCCEEDED(d3d_debug.as(d3d_info_queue))) {

            d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

            D3D11_MESSAGE_ID hide[] = {
               D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
            };

            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3d_info_queue->AddStorageFilterEntries(&filter);
         }
      }
   }

   throw_if_failed(device.as(_device));
   throw_if_failed(context.as(_device_context));
}

void Editor::create_swap_chain()
{
   Com_ptr<IDXGIDevice1> dxgi_device;
   throw_if_failed(_device.as(dxgi_device));

   Com_ptr<IDXGIAdapter> adapater;
   throw_if_failed(dxgi_device->GetAdapter(adapater.clear_and_assign()));

   Com_ptr<IDXGIFactory2> factory;
   throw_if_failed(adapater->GetParent(IID_PPV_ARGS(factory.clear_and_assign())));

   DXGI_SWAP_CHAIN_DESC1 desc{};
   desc.Width = _window_size.x;
   desc.Height = _window_size.y;
   desc.Format = _backbuffer_format;
   desc.SampleDesc.Count = 1;
   desc.SampleDesc.Quality = 0;
   desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   desc.BufferCount = 2;

   DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc{};
   fsSwapChainDesc.Windowed = true;

   throw_if_failed(factory->CreateSwapChainForHwnd(_device.get(), _window, &desc,
                                                   &fsSwapChainDesc, nullptr,
                                                   _swap_chain.clear_and_assign()));

   throw_if_failed(factory->MakeWindowAssociation(_window, DXGI_MWA_NO_ALT_ENTER));

   Com_ptr<ID3D11Texture2D> bacK_buffer;

   throw_if_failed(
      _swap_chain->GetBuffer(0, IID_PPV_ARGS(bacK_buffer.clear_and_assign())));
   throw_if_failed(_device->CreateRenderTargetView(bacK_buffer.get(), nullptr,
                                                   _bacK_buffer.clear_and_assign()));
}

}
