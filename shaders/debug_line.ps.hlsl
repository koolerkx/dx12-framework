struct PSIN {
  float4 position : SV_POSITION;
  float4 color : BASE_COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  return input.color;
}
