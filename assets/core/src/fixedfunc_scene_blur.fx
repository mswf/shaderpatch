
#include "constants_list.hlsl"
#include "pixel_sampler_states.hlsl"

Texture2D<float3> source_texture : register(t0);

struct Vs_input {
   float4 position : POSITIONT;
   float2 texcoords : TEXCOORD;
};

struct Vs_output {
   float2 texcoords : TEXCOORD;
   nointerpolation float alpha : ALPHA;
   float4 positionPS : SV_Position;
};

Vs_output main_vs(Vs_input input)
{
   Vs_output output;

   const float2 position = 
      ((input.position.xy * ff_inv_resolution) - 0.5) * float2(2.0, -2.0);

   output.positionPS = float4(position, 0.0, 1.0);
   output.alpha = ff_texture_factor.a;
   output.texcoords = input.texcoords;
   
   return output;
}

struct Ps_input {
   float2 texcoords : TEXCOORD;
   nointerpolation float alpha : ALPHA;
};

float4 main_ps(Ps_input input) : SV_Target0
{
   return float4(source_texture.SampleLevel(linear_clamp_sampler, input.texcoords, 0), input.alpha);
}
