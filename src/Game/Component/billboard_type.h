#pragma once
#include <cstdint>

namespace Billboard {

enum class Mode : uint8_t {
  None = 0,       // No billboard, use original rotation
  Cylindrical,    // Y-axis rotation only (for trees, particles)
  Spherical       // Full rotation facing camera
};

}  // namespace Billboard
