cbuffer FrameCB : register(b0) {
  row_major float4x4 g_View;
  row_major float4x4 g_Proj;
  row_major float4x4 g_ViewProj;
  float3 g_CameraPos;
  float g_Time;
  float2 g_ScreenSize;
  float2 _padding0;
};

cbuffer ObjectCB : register(b1) {
  row_major float4x4 g_World;
  row_major float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
  float2 g_UVOffset; // UV offset (unused in debug lines)
  float2 g_UVScale;  // UV scale (unused in debug lines)
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
  // g_WorldViewProj contains the view_proj matrix passed from
  // debug_line_renderer
  output.position = mul(float4(input.position, 1.0f), g_WorldViewProj);
  output.color = input.color;

  return output;
}
