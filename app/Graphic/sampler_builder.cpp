#include "sampler_builder.h"

namespace SamplerPresets {

SamplerBuilder CreatePointSampler(UINT shaderRegister, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility) {
  return SamplerBuilder(shaderRegister)
    .SetFilter(D3D12_FILTER_MIN_MAG_MIP_POINT)
    .SetAddressMode(addressMode)
    .SetBorderColor(D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK)
    .SetLODRange(0.0f, D3D12_FLOAT32_MAX)
    .SetComparisonFunc(D3D12_COMPARISON_FUNC_NEVER)
    .SetShaderVisibility(visibility)
    .SetMaxAnisotropy(0);
}

SamplerBuilder CreateLinearSampler(UINT shaderRegister, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility) {
  return SamplerBuilder(shaderRegister)
    .SetFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
    .SetAddressMode(addressMode)
    .SetShaderVisibility(visibility);
}

SamplerBuilder CreateAnisotropicSampler(
  UINT shaderRegister, UINT maxAnisotropy, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility) {
  return SamplerBuilder(shaderRegister)
    .SetFilter(D3D12_FILTER_ANISOTROPIC)
    .SetMaxAnisotropy(maxAnisotropy)
    .SetAddressMode(addressMode)
    .SetShaderVisibility(visibility);
}

SamplerBuilder CreateComparisonSampler(UINT shaderRegister, D3D12_COMPARISON_FUNC comparisonFunc, D3D12_SHADER_VISIBILITY visibility) {
  return SamplerBuilder(shaderRegister)
    .SetFilter(D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR)
    .SetComparisonFunc(comparisonFunc)
    .SetAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER)
    .SetBorderColor(D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE)
    .SetShaderVisibility(visibility);
}

}  // namespace SamplerPresets
