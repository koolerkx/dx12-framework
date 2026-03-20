#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Framework/Math/Math.h"
#include "Framework/Model/node_hierarchy.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/texture_handle.h"

struct ModelSurfaceMaterial {
  DirectX::XMFLOAT4 base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
  DirectX::XMFLOAT3 emissive_factor = {0.0f, 0.0f, 0.0f};
  float metallic_factor = 0.0f;
  float roughness_factor = 0.5f;
  bool is_double_sided = false;
};

struct ModelSubMeshEntry {
  MeshHandle mesh_handle;
  TextureHandle albedo_texture;
  TextureHandle normal_texture;
  TextureHandle metallic_roughness_texture;
  TextureHandle emissive_texture;
  uint32_t surface_material_index = 0;
};

struct ModelData {
  std::string path;
  std::vector<std::shared_ptr<void>> resource_refs_;  // keeps loaded textures alive
  std::vector<ModelSurfaceMaterial> surface_materials;
  std::vector<ModelSubMeshEntry> sub_meshes;
  Model::Node root_node;
  float min_y = 0.0f;
  Math::AABB bounds;
};
