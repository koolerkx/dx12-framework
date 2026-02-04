#pragma once
#include <DirectXMath.h>

#include <bit>
#include <variant>
#include <vector>

#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"
#include "camera_data.h"

struct Texture;
class Mesh;

enum class RenderPassTag {
  Ui,                // Used for HUD, Menus (Overlay)
  WorldOpaque,       // Used for solid sprites in the world
  WorldTransparent,  // Used for alpha-blended sprites with depth
  Debug              // Used for engine tools/gizmos
};

using SpriteInstanceData = Graphics::Vertex::SpriteInstance;

// Common sort key for both single and instance draw commands
struct DrawSortKey {
  // Key: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
  [[nodiscard]] uint64_t GetSortKeyForOpaque(const Material* material, float depth, bool front_to_back) const {
    if (!material) {
      return UINT64_MAX;
    }

    uint32_t rs_key = material->GetRootSignatureKey() & 0xFFFF;
    uint32_t pso_key = material->GetPSOKey() & 0xFFFF;
    uint64_t compressed_material = (static_cast<uint64_t>(rs_key) << 48) | (static_cast<uint64_t>(pso_key) << 32);

    uint32_t depth_key = std::bit_cast<uint32_t>(depth);
    if (!front_to_back) {
      depth_key = ~depth_key;
    }

    return compressed_material | depth_key;
  }

  // Key: [Depth 32-bit | RS hash 16-bit | PSO hash 16-bit]
  [[nodiscard]] uint64_t GetSortKeyForTransparent(const Material* material, float depth, bool front_to_back) const {
    if (!material) {
      return UINT64_MAX;
    }

    uint32_t depth_key = std::bit_cast<uint32_t>(depth);
    if (!front_to_back) {
      depth_key = ~depth_key;
    }

    uint32_t rs_key = material->GetRootSignatureKey() & 0xFFFF;
    uint32_t pso_key = material->GetPSOKey() & 0xFFFF;
    uint64_t material_key = (static_cast<uint64_t>(rs_key) << 16) | static_cast<uint64_t>(pso_key);

    return (static_cast<uint64_t>(depth_key) << 32) | material_key;
  }

  [[nodiscard]] uint64_t GetSortKeyWithDepth(const Material* material, float depth, bool front_to_back, bool depth_first) const {
    return depth_first ? GetSortKeyForTransparent(material, depth, front_to_back) : GetSortKeyForOpaque(material, depth, front_to_back);
  }
};

// For single object rendering (SpriteRenderer, MeshRenderer)
struct SingleDrawCommand : DrawSortKey {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix{};
  DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};
  MaterialInstance material_instance{};
  float depth = 0.0f;
  DirectX::XMFLOAT2 uv_offset{0.0f, 0.0f};
  DirectX::XMFLOAT2 uv_scale{1.0f, 1.0f};
};

// For instanced rendering (TextRenderer, ParticleRenderer)
struct InstanceDrawCommand : DrawSortKey {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  MaterialInstance material_instance;
  float depth = 0.0f;
  std::vector<SpriteInstanceData> instances;
  int layer_id = 0;
};

// Variant type for unified storage and global sorting
using DrawCommandVariant = std::variant<SingleDrawCommand, InstanceDrawCommand>;

// The packet sent from Scene to Renderer
struct FramePacket {
  CameraData main_camera;

  // New unified command vectors for global sorting
  std::vector<DrawCommandVariant> opaque_pass;
  std::vector<DrawCommandVariant> transparent_pass;
  std::vector<DrawCommandVariant> ui_pass;

  void Clear() {
    opaque_pass.clear();
    transparent_pass.clear();
    ui_pass.clear();
  }
};
