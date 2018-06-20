
#include "com_ptr.hpp"
#include "logger.hpp"

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <gsl/gsl>

#include <d3d9.h>

namespace sp {

template<typename Processor>
class Resource_handle final : public IDirect3DVolumeTexture9 {
public:
   static_assert(std::is_invocable_v<Processor, gsl::span<std::byte>>,
                 "Resource processor must be invocable with "
                 "gsl::span<std::byte> as an argument.");

   static_assert(std::is_invocable_r_v<void, Processor, gsl::span<std::byte>>,
                 "Resource processor must be not have void return type.");

   Resource_handle(std::uint32_t resource_size, Processor processor)
      : _resource_size{resource_size}, _processor{std::move(processor)}
   {
   }

   Resource_handle(const Resource_handle&) = delete;
   Resource_handle& operator=(const Resource_handle&) = delete;

   Resource_handle(Resource_handle&&) = delete;
   Resource_handle& operator=(Resource_handle&&) = delete;

   HRESULT __stdcall LockBox(UINT level, D3DLOCKED_BOX* locked_volume,
                             const D3DBOX* box, DWORD) noexcept override
   {
      if (level != 0) return D3DERR_INVALIDCALL;
      if (locked_volume == nullptr) return D3DERR_INVALIDCALL;
      if (box != nullptr) return D3DERR_INVALIDCALL;
      if (_data) return D3DERR_INVALIDCALL;

      _data = std::make_unique<std::byte[]>(_resource_size);

      locked_volume->pBits = _data.get();
      locked_volume->RowPitch = -1;
      locked_volume->SlicePitch = -1;

      return S_OK;
   }

   HRESULT __stdcall UnlockBox(UINT level) noexcept override
   {
      if (level != 0) return D3DERR_INVALIDCALL;
      if (!_data) return D3DERR_INVALIDCALL;

      _keep_alive = _processor(gsl::make_span(_data.get(), _resource_size));

      _data.reset();

      return S_OK;
   }

   HRESULT __stdcall QueryInterface(const IID& iid, void** obj) noexcept override
   {
      if (iid == IID_IDirect3DVolumeTexture9) {
         *obj = static_cast<IDirect3DVolumeTexture9*>(this);
      }
      else if (iid == IID_IDirect3DBaseTexture9) {
         *obj = static_cast<IDirect3DBaseTexture9*>(this);
      }
      else if (iid == IID_IDirect3DResource9) {
         *obj = static_cast<IDirect3DResource9*>(this);
      }
      else if (iid == IID_IUnknown) {
         *obj = static_cast<IUnknown*>(this);
      }
      else {
         return E_NOINTERFACE;
      }

      AddRef();

      return S_OK;
   }

   ULONG __stdcall AddRef() noexcept override
   {
      return _ref_count += 1;
   }

   ULONG __stdcall Release() noexcept override
   {
      const auto ref_count = _ref_count -= 1;

      if (ref_count == 0) delete this;

      return ref_count;
   }

   HRESULT __stdcall GetDevice(IDirect3DDevice9**) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   HRESULT __stdcall SetPrivateData(const GUID&, const void*, DWORD, DWORD) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   HRESULT __stdcall GetPrivateData(const GUID&, void*, DWORD*) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   HRESULT __stdcall FreePrivateData(const GUID&) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   DWORD __stdcall SetPriority(DWORD) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return 0;
   }

   DWORD __stdcall GetPriority() noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return 0;
   }

   void __stdcall PreLoad() noexcept override {}

   D3DRESOURCETYPE __stdcall GetType() noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return D3DRTYPE_VOLUMETEXTURE;
   }

   DWORD __stdcall SetLOD(DWORD) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return 0;
   }

   DWORD __stdcall GetLOD() noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return 0;
   }

   DWORD __stdcall GetLevelCount() noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return 1;
   }

   HRESULT __stdcall SetAutoGenFilterType(D3DTEXTUREFILTERTYPE) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return S_OK;
   }

   D3DTEXTUREFILTERTYPE __stdcall GetAutoGenFilterType() noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return D3DTEXF_NONE;
   }

   void __stdcall GenerateMipSubLevels() noexcept override {}

   HRESULT __stdcall GetLevelDesc(UINT, D3DVOLUME_DESC*) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   HRESULT __stdcall GetVolumeLevel(UINT, IDirect3DVolume9**) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

   HRESULT __stdcall AddDirtyBox(const D3DBOX*) noexcept override
   {
      log(Log_level::warning,
          "Unimplemented function \"" __FUNCSIG__ "\" called.");

      return E_NOTIMPL;
   }

private:
   ~Resource_handle() = default;

   const std::uint32_t _resource_size;
   Processor _processor;

   std::unique_ptr<std::byte[]> _data;

   std::invoke_result_t<Processor, gsl::span<std::byte>> _keep_alive;
   std::atomic<ULONG> _ref_count{1};
};

inline auto make_resource_uploader(std::uint32_t resource_size,
                                   std::function<void(gsl::span<std::byte>)> processor) noexcept
   -> Com_ptr<IDirect3DVolumeTexture9>
{
   Expects(processor != nullptr);

   const auto processor_wrapper = [processor](gsl::span<std::byte> span) {
      processor(span);

      return nullptr;
   };

   return Com_ptr<IDirect3DVolumeTexture9>{
      new Resource_handle<decltype(processor_wrapper)>{resource_size, processor_wrapper}};
}

template<typename Processor>
inline auto make_resource_handler(std::uint32_t resource_size, Processor processor) noexcept
   -> Com_ptr<IDirect3DVolumeTexture9>
{
   return Com_ptr<IDirect3DVolumeTexture9>{
      new Resource_handle<Processor>{resource_size, std::move(processor)}};
}
}