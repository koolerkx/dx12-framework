#pragma once

#include <cstdint>

namespace ShadowCascadeConfig {

[[maybe_unused]] constexpr uint32_t MAX_CASCADES = 4;
[[maybe_unused]] constexpr uint32_t DEFAULT_CASCADE_COUNT = 3;
[[maybe_unused]] constexpr float SPLIT_LAMBDA = 0.75f;

}  // namespace ShadowCascadeConfig

// Hardware rasterizer bias applied during shadow map rendering (caster-side).
// These are baked into the PSO and require pipeline rebuild to change.
// Tuning guide:
//   DEPTH_BIAS: fixed offset in D32_FLOAT minimum-resolvable units
//   SLOPE_SCALED_DEPTH_BIAS: multiplied by the surface slope (handles grazing angles)
//   DEPTH_BIAS_CLAMP: caps the total hardware bias to prevent peter panning
namespace ShadowHardwareConfig {

[[maybe_unused]] constexpr int32_t DEPTH_BIAS = 1500;
[[maybe_unused]] constexpr float SLOPE_SCALED_DEPTH_BIAS = 1.5f;
[[maybe_unused]] constexpr float DEPTH_BIAS_CLAMP = 0.01f;

}  // namespace ShadowHardwareConfig

// Receiver-side bias defaults, adjustable at runtime via ImGui.
// These are passed to the pixel shader through ShadowCB.
//   DEPTH_BIAS: small epsilon for depth comparison (keep near 0 when hardware bias is active)
//   NORMAL_BIAS: offsets sample position along surface normal to reduce self-shadowing
namespace ShadowReceiverDefaults {

[[maybe_unused]] constexpr float DEPTH_BIAS = 0.0001f;
[[maybe_unused]] constexpr float NORMAL_BIAS = 0.05f;

}  // namespace ShadowReceiverDefaults
