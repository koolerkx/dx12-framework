#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Framework/Math/Math.h"
#include "Framework/Model/node_hierarchy.h"
#include "Graphic/Resource/Mesh/mesh_buffer_pool.h"
#include "Graphic/Resource/Texture/texture.h"
#include "Graphic/Resource/mesh.h"

struct ModelSurfaceMaterial {
  DirectX::XMFLOAT4 base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
  DirectX::XMFLOAT3 emissive_factor = {0.0f, 0.0f, 0.0f};
  float metallic_factor = 0.0f;
  float roughness_factor = 0.5f;
  bool is_double_sided = false;
};

struct ModelSubMeshEntry {
  const Mesh* mesh = nullptr;  // DEPRECATED(Phase4): Remove after bindless migration complete
  MeshHandle mesh_handle;
  std::shared_ptr<Texture> albedo_texture;
  std::shared_ptr<Texture> normal_texture;
  std::shared_ptr<Texture> metallic_roughness_texture;
  std::shared_ptr<Texture> emissive_texture;
  uint32_t surface_material_index = 0;
};

struct ModelData {
  std::string path;
  std::vector<std::shared_ptr<Texture>> textures;
  std::vector<ModelSurfaceMaterial> surface_materials;
  std::vector<ModelSubMeshEntry> sub_meshes;
  Model::Node root_node;
  float min_y = 0.0f;
  Math::AABB bounds;
};
