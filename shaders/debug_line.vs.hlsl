cbuffer FrameCB : register(b0) {
  float4x4 g_View;
  float4x4 g_Proj;
  float4x4 g_ViewProj;
  float3 g_CameraPos;
  float g_Time;
  float2 g_ScreenSize;
  float2 _padding0;
};

cbuffer ObjectCB : register(b1) {
  float4x4 g_World;
  float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
};

struct VSInput {
  float3 position : POSITION;
  float4 color : COLOR;
};

struct PSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
};

PSInput main(VSInput input) {
  PSInput output;

  // Transform position directly with pre-multiplied matrix
  // g_WorldViewProj contains the view_proj matrix passed from debug_line_renderer
  output.position = mul(float4(input.position, 1.0f), g_WorldViewProj);
  output.color = input.color;

  return output;
}
