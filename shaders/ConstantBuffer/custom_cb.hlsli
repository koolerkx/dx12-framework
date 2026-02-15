#ifndef CUSTOM_CB_HLSLI
#define CUSTOM_CB_HLSLI

// Slot definition only. Each pixel shader defines its own typed struct
// and binds to this register for readability and semantic naming.
//
// Usage in .ps.hlsl:
//   #include "ConstantBuffer/custom_cb.hlsli"
//   struct ScanlineCB { float3 primaryColor; float gridDivisions; ... };
//   ConstantBuffer<ScanlineCB> g_CustomCB : register(CUSTOM_CB_SLOT);

#ifndef CUSTOM_CB_SLOT
#define CUSTOM_CB_SLOT b5
#endif

// struct CustomCB {
//   float4 data[4];
// }
// ConstantBuffer<ScanlineCB> g_CustomCB : register(CUSTOM_CB_SLOT);

#endif
