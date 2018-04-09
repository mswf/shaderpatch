#pragma once

#include <cstddef>

#include <d3d9.h>
#include <dxgiformat.h>

namespace sp {

inline D3DFORMAT dxgi_format_to_d3dformat(DXGI_FORMAT dxgi_format)
{
   switch (dxgi_format) {
   case DXGI_FORMAT_B8G8R8A8_UNORM:
   case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      return D3DFMT_A8R8G8B8;
   case DXGI_FORMAT_B8G8R8X8_UNORM:
   case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
      return D3DFMT_X8R8G8B8;
   case DXGI_FORMAT_B5G6R5_UNORM:
      return D3DFMT_R5G6B5;
   case DXGI_FORMAT_B5G5R5A1_UNORM:
      return D3DFMT_A1R5G5B5;
   case DXGI_FORMAT_B4G4R4A4_UNORM:
      return D3DFMT_A4R4G4B4;
   case DXGI_FORMAT_A8_UNORM:
      return D3DFMT_A8;
   case DXGI_FORMAT_R10G10B10A2_UNORM:
      return D3DFMT_A2B10G10R10;
   case DXGI_FORMAT_R8G8B8A8_UNORM:
   case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      return D3DFMT_A8B8G8R8;
   case DXGI_FORMAT_R16G16_UNORM:
      return D3DFMT_G16R16;
   case DXGI_FORMAT_R16G16B16A16_UNORM:
      return D3DFMT_A16B16G16R16;
   case DXGI_FORMAT_R8_UNORM:
      return D3DFMT_L8;
   case DXGI_FORMAT_R8G8_SNORM:
      return D3DFMT_V8U8;
   case DXGI_FORMAT_R8G8B8A8_SNORM:
      return D3DFMT_Q8W8V8U8;
   case DXGI_FORMAT_R16G16_SNORM:
      return D3DFMT_V16U16;
   case DXGI_FORMAT_G8R8_G8B8_UNORM:
      return D3DFMT_R8G8_B8G8;
   case DXGI_FORMAT_R8G8_B8G8_UNORM:
      return D3DFMT_G8R8_G8B8;
   case DXGI_FORMAT_BC1_UNORM:
   case DXGI_FORMAT_BC1_UNORM_SRGB:
      return D3DFMT_DXT1;
   case DXGI_FORMAT_BC2_UNORM:
   case DXGI_FORMAT_BC2_UNORM_SRGB:
      return D3DFMT_DXT3;
   case DXGI_FORMAT_BC3_UNORM:
   case DXGI_FORMAT_BC3_UNORM_SRGB:
      return D3DFMT_DXT5;
   case DXGI_FORMAT_BC4_UNORM:
      return static_cast<D3DFORMAT>(MAKEFOURCC('A', 'T', 'I', '1'));
   case DXGI_FORMAT_R16_UNORM:
      return D3DFMT_L16;
   case DXGI_FORMAT_R16G16B16A16_SNORM:
      return D3DFMT_Q16W16V16U16;
   case DXGI_FORMAT_R16_FLOAT:
      return D3DFMT_R16F;
   case DXGI_FORMAT_R16G16_FLOAT:
      return D3DFMT_G16R16F;
   case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return D3DFMT_A16B16G16R16F;
   case DXGI_FORMAT_R32_FLOAT:
      return D3DFMT_R32F;
   case DXGI_FORMAT_R32G32_FLOAT:
      return D3DFMT_G32R32F;
   case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return D3DFMT_A32B32G32R32F;
   default:
      return D3DFMT_UNKNOWN;
   }
}
}
