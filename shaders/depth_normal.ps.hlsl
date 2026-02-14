struct PSInput {
    float4 position   : SV_POSITION;
    float3 viewNormal : TEXCOORD0;
    float  viewDepth  : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET {
    return float4(normalize(input.viewNormal), input.viewDepth);
}
