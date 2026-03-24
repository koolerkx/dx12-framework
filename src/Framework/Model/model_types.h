#pragma once

#include <DirectXMath.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Model {

/**
 * @brief Texture type for proper color space handling
 *
 * Color textures (albedo, diffuse) should use sRGB
 * Data textures (normal, roughness, metallic) should use linear
 */
enum class TextureType {
  Color,   // Albedo, diffuse - use sRGB
  Data,    // Normal, roughness, metallic, height - use linear
  Unknown  // Let system decide
};

/**
 * @brief Embedded texture data stored within the model file
 *
 * Some formats like glTF can embed textures directly in the file.
 */
struct EmbeddedTexture {
  std::vector<uint8_t> data;
  uint32_t width = 0;
  uint32_t height = 0;
  std::string format_hint;  // "png", "jpg", "rgba8888", etc.
  bool is_compressed = false;
};

/**
 * @brief Texture reference that can be either external file or embedded data
 */
struct TextureRef {
  std::string path;                         // External file path (empty if embedded)
  std::optional<EmbeddedTexture> embedded;  // Embedded texture data
  TextureType type = TextureType::Unknown;
  bool has_alpha = false;  // Hint for alpha channel presence
};

/**
 * @brief Surface material properties from 3D model files
 *
 * This represents the surface material data from Assimp/GLTF/OBJ files.
 * @note This is NOT the DX12 Material (which is PSO + Root Signature).
 *
 * @warning Texture indices reference LoadResult::texture_handles.
 * This is set during Stage 1 processing, after texture_callback is invoked.
 *
 * The flow is:
 * 1. ModelLoader collects all unique texture paths from materials
 * 2. For each unique texture, texture_callback is called (automatic deduplication)
 * 3. Texture handles are stored in LoadResult::texture_handles
 * 4. SurfaceMaterialData gets indices into texture_handles (not TextureRef paths)
 * 5. User can directly use texture_handles[index] for rendering
 */
struct SurfaceMaterialData {
  std::string name;

  // PBR material factors
  DirectX::XMFLOAT4 base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
  float metallic_factor = 0.0f;
  float roughness_factor = 0.5f;
  DirectX::XMFLOAT3 emissive_factor = {0.0f, 0.0f, 0.0f};
  float alpha_cutoff = 0.5f;

  // Texture indices (indices into LoadResult::texture_handles)
  // std::nullopt means this texture type is not present
  std::optional<uint32_t> base_color_texture_index;          // Albedo/diffuse
  std::optional<uint32_t> normal_texture_index;              // Normal map
  std::optional<uint32_t> metallic_roughness_texture_index;  // PBR metallic+roughness packed
  std::optional<uint32_t> emissive_texture_index;            // Emissive
  std::optional<uint32_t> occlusion_texture_index;           // Ambient occlusion

  // Material flags
  bool has_vertex_colors = false;
  bool is_double_sided = false;
  bool is_alpha_test = false;   // Uses alpha cutoff
  bool is_alpha_blend = false;  // Uses alpha blending
};

/**
 * @brief Node transformation data
 *
 * Stores translation, rotation, and scale separately,
 * plus the computed local transform matrix.
 */
struct NodeTransform {
  DirectX::XMFLOAT3 translation = {0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT4 rotation = {0.0f, 0.0f, 0.0f, 1.0f};  // Quaternion
  DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};

  // Computed local transform matrix (translation * rotation * scale)
  DirectX::XMFLOAT4X4 local_matrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
};

template <typename T>
concept VertexWithPosition = requires(T vertex) {
  { vertex.position };
};

template <typename T>
concept VertexWithNormal = requires(T vertex) {
  { vertex.normal };
};

template <typename T>
concept VertexWithTangent = requires(T vertex) {
  { vertex.tangent };
};

template <typename T>
concept VertexWithTexcoord = requires(T vertex) {
  { vertex.texcoord };
};

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
