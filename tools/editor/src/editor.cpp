
#include "editor.hpp"
#include "debug_state.hpp"
#include "file_dialogs.hpp"
#include "imgui_impl_dx11.h"
#include "synced_io.hpp"
#include "throw_if_failed.hpp"

#include <string>
#include <string_view>

using namespace std::literals;

namespace sp::editor {

Editor::Editor(HWND window)
   : _window{window}, _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext}
{
   if (_editor_created.exchange(true)) std::terminate();

   RECT rect{};
   GetClientRect(window, &rect);

   _window_size = {rect.right - rect.left, rect.bottom - rect.top};

   create_device();
   create_swap_chain();
   create_resources();
   init_imgui_impl();
}

void Editor::update()
{
   constexpr float colour[] = {0.0f, 0.0f, 0.0f, 1.0f};
   _device_context->ClearRenderTargetView(_bacK_buffer.get(), colour);

   ImGui_ImplDX11_NewFrame();

   if (_project_unopened) open_project();

   ImGui::Render();
   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

   if (const auto hr = _swap_chain->Present(1, 0);
       hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
      handle_device_lost();
   }
   else {
      throw_if_failed(hr);
   }
}

void Editor::window_size_changed(glm::ivec2 size) noexcept
{
   _window_size = size;
   _bacK_buffer = nullptr;

   _swap_chain->ResizeBuffers(2, _window_size.x, _window_size.y, _backbuffer_format, 0);
   create_resources();
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
      D3D_FEATURE_LEVEL_11_0,
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

   DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_chain_desc{};
   swap_chain_desc.Windowed = true;

   throw_if_failed(factory->CreateSwapChainForHwnd(_device.get(), _window, &desc,
                                                   &swap_chain_desc, nullptr,
                                                   _swap_chain.clear_and_assign()));
   throw_if_failed(factory->MakeWindowAssociation(_window, DXGI_MWA_NO_ALT_ENTER));
}

void Editor::create_resources()
{
   Com_ptr<ID3D11Texture2D> bacK_buffer;

   throw_if_failed(
      _swap_chain->GetBuffer(0, IID_PPV_ARGS(bacK_buffer.clear_and_assign())));
   throw_if_failed(_device->CreateRenderTargetView(bacK_buffer.get(), nullptr,
                                                   _bacK_buffer.clear_and_assign()));

   auto view = {_bacK_buffer.get()};
   _device_context->OMSetRenderTargets(view.size(), view.begin(), nullptr);
}

void Editor::init_imgui_impl()
{
   ImGui_ImplDX11_Init(_window, _device.get(), _device_context.get());
   ImGui_ImplDX11_CreateDeviceObjects();
}

void Editor::handle_device_lost()
{
   _bacK_buffer = nullptr;
   _swap_chain = nullptr;
   _device_context = nullptr;
   _device = nullptr;

   ImGui_ImplDX11_Shutdown();

   create_device();
   create_swap_chain();
   create_resources();
   init_imgui_impl();
}

void Editor::open_project()
{
   ImGui::SetNextWindowPos(glm::vec2{_window_size} * 0.5f, ImGuiCond_Always,
                           {0.5f, 0.5f});
   ImGui::Begin("Projects", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

   if (ImGui::Button("Open Project")) {
      if (auto path = win32::folder_dialog(); path) {
         _project = Project{*path};
         _project_unopened = false;

         std::wstring win_name = window_title;
         win_name += L" - "sv;
         win_name += path->stem().native();

         SetWindowTextW(_window, win_name.c_str());
      }
   }

   ImGui::Text("Recent Projects");
   ImGui::Separator();

   ImGui::End();
}
}
