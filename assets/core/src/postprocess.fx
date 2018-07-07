
#include "fullscreen_tri_vs.hlsl"

#pragma warning(disable : 3571)

sampler2D scene_sampler : register(s0);
sampler2D bloom_stages[5] : register(s1);
sampler2D dirt_sampler : register(s6);
sampler2D red_lut : register(s7);
sampler2D green_lut : register(s8);
sampler2D blue_lut : register(s9);

float4 bloom_global_scale_threshold : register(c61);
float3 bloom_local_scales[5] : register(c62);
float3 bloom_dirt_scale : register(c67);
float4 exposure_color_filter_saturation : register(c68);

const static float3 luma_weights = {0.2126, 0.7152, 0.0722};
const static float3 fxaa_luma_weights = {0.299, 0.587, 0.114};
const static float3 saturation_weights = {0.25, 0.5, 0.25};

const static float3 bloom_global_scale = bloom_global_scale_threshold.xyz;
const static float bloom_threshold = bloom_global_scale_threshold.w;

const static float3 exposure_color_filter = exposure_color_filter_saturation.xyz;
const static float saturation = exposure_color_filter_saturation.w;

const static bool bloom = BLOOM_ACTIVE;
const static bool bloom_use_dirt = BLOOM_USE_DIRT;
const static bool stock_hdr = STOCK_HDR_STATE;

float3 combine_bloom(float2 texcoords)
{
   float3 color = 0.0;

   [unroll] for (int i = 0; i < 5; ++i) {
      color += tex2D(bloom_stages[i], texcoords).rgb * bloom_local_scales[i];
   }
   
   if (bloom_use_dirt) {
      color += color * (tex2D(dirt_sampler, texcoords).rgb * bloom_dirt_scale);
   }
   
   return color * bloom_global_scale;
}

float sample_lut(sampler2D lut, float v)
{
   return tex2D(lut, float2(sqrt(v) + 1.0 / 256.0, 0.0)).x;
}

float3 apply_color_grading(float3 color)
{
   color *= exposure_color_filter;

   float grey = dot(saturation_weights, color);
   color = grey + (saturation * (color - grey));

   color.r = sample_lut(red_lut, color.r);
   color.g = sample_lut(green_lut, color.g);
   color.b = sample_lut(blue_lut, color.b);

   return color;
}

float fxaa_luma(float3 color)
{
   return sqrt(dot(color, fxaa_luma_weights));
}

float4 postprocess_ps(float2 texcoords : TEXCOORD) : COLOR
{
   float3 color = tex2D(scene_sampler, texcoords).rgb;

   if (stock_hdr) color = pow(color + color, 2.2);

   if (bloom) color += combine_bloom(texcoords);

   color = apply_color_grading(color);

   return float4(color, fxaa_luma(color));
}

float4 bloom_threshold_ps(float2 texcoords : TEXCOORD) : COLOR
{
   float3 color = tex2D(scene_sampler, texcoords).rgb;
   
   if (dot(stock_hdr ? fxaa_luma_weights : luma_weights, color) < bloom_threshold) return 0.0;

   return float4(color, 1.0);
}
