#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "model_concepts.h"

namespace Model {

template <typename VertexType>
struct MeshData {
  std::string name;
  std::vector<VertexType> vertices;
  std::vector<uint32_t> indices;
  uint32_t material_index = 0;

};

struct LoadOptions {
  float global_scale = 1.0f;
  bool load_animations = false;
  bool generate_tangents = true;
  bool combine_meshes_by_material = false;
  bool generate_smooth_normals = false;
  float normal_smoothing_angle = 60.0f;
};

}  // namespace Model
