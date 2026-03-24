#pragma once

#include <DirectXMath.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/texture.h>

#include <assimp/Importer.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "model_types.h"
#include "node_hierarchy.h"

namespace Model {

/**
 * @brief Template-based model loader using Assimp with two-stage callback pattern
 *
 * This follows Assimp's recommended "Material-First" processing pipeline:
 * - Stage 1: Process all materials and textures (with automatic deduplication)
 * - Stage 2: Process meshes (which reference pre-loaded materials)
 *
 * @tparam MeshHandle User-defined mesh handle type (e.g., std::shared_ptr<Mesh>, uint32_t)
 * @tparam TextureHandle User-defined texture handle type (e.g., AssetHandle<Texture>, uint32_t)
 * @tparam SurfaceMaterialHandle User-defined surface material handle type
 *
 * Processing flow:
 * 1. Collect all materials from aiScene
 * 2. Extract all unique texture paths from materials
 * 3. Call texture_callback for each unique texture (automatic deduplication)
 * 4. Create SurfaceMaterialData with texture handle indices
 * 5. Call surface_material_callback for each material
 * 6. Process meshes with mesh_callback
 * 7. Build scene node hierarchy
 *
 * Example usage:
 * @code
 * struct PBRVertex {
 *   float position[3];
 *   float normal[3];
 *   float tangent[4];
 *   float texcoord[2];
 * };
 *
 * auto result = Model::ModelLoader<uint32_t, uint32_t, uint32_t>::Load<PBRVertex>(
 *   "models/character.fbx",
 *   // Stage 1: Handle textures
 *   [&](const Model::TextureRef& tex_ref) -> uint32_t {
 *     return texture_manager.Load(tex_ref.path, tex_ref.type == Model::TextureType::Color);
 *   },
 *   // Stage 1: Handle materials
 *   [&](const Model::SurfaceMaterialData& mat) -> uint32_t {
 *     uint32_t albedo_idx = mat.base_color_texture_index.value_or(UINT32_MAX);
 *     return CreateMaterial(mat.base_color_factor, albedo_idx, mat.is_double_sided);
 *   },
 *   // Stage 2: Handle meshes
 *   [&](const Model::MeshData<PBRVertex>& mesh) -> uint32_t {
 *     return CreateMesh(mesh.vertices, mesh.indices, mesh.material_index);
 *   }
 * );
 *
 * // Use the result:
 * // result.texture_handles - all loaded textures (deduplicated)
 * // result.surface_material_handles - all materials (with texture indices)
 * // result.mesh_handles - all meshes
 * // result.root_node - scene hierarchy
 * @endcode
 */
template <typename MeshHandle, typename TextureHandle, typename SurfaceMaterialHandle>
class ModelLoader {
 private:
  // Internal helper for texture key (used in Load function)
  struct TextureKey {
    std::string path;
    TextureType type;

    bool operator==(const TextureKey& other) const {
      return path == other.path && type == other.type;
    }
  };

  struct TextureKeyHash {
    std::size_t operator()(const TextureKey& key) const {
      std::size_t seed = std::hash<std::string>()(key.path);
      seed ^= std::hash<int>()(static_cast<int>(key.type)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

 public:
  // === Callback Type Definitions ===

  /**
   * @brief Texture loading callback (Stage 1)
   *
   * Called for each UNIQUE texture. The ModelLoader automatically deduplicates
   * textures, so this callback is only called once per unique texture path.
   *
   * @param tex_ref Texture reference with path, type, and embedded data
   * @return TextureHandle Handle to the loaded texture
   */
  using TextureCallback = std::function<TextureHandle(const TextureRef&)>;

  /**
   * @brief Surface material creation callback (Stage 1)
   *
   * Called for each material AFTER texture_callback has been invoked.
   * The SurfaceMaterialData contains texture HANDLE INDICES (not paths),
   * which can be used directly to reference LoadResult::texture_handles.
   *
   * @param material Material data with texture handle indices
   * @return SurfaceMaterialHandle Handle to the created material
   */
  using SurfaceMaterialCallback = std::function<SurfaceMaterialHandle(const SurfaceMaterialData&)>;

  /**
   * @brief Mesh creation callback (Stage 2)
   *
   * Called for each mesh AFTER all materials and textures are loaded.
   * Each mesh has a material_index that references SurfaceMaterialData.
   *
   * @tparam VertexType User-defined vertex structure
   * @param mesh Mesh data with vertices, indices, and material_index
   * @return MeshHandle Handle to the created mesh
   */
  template <typename VertexType>
  using MeshCallback = std::function<MeshHandle(const MeshData<VertexType>&)>;

  /**
   * @brief Bone loading callback (reserved for future animation system)
   */
  using BoneCallback = std::function<void(const std::vector<BoneData>&)>;

  /**
   * @brief Animation loading callback (reserved for future animation system)
   */
  using AnimationCallback = std::function<void(const AnimationData&)>;

  /**
   * @brief Result of model loading operation
   *
   * The order of handles matters: texture_handles are populated first (Stage 1),
   * then surface_material_handles (Stage 1), then mesh_handles (Stage 2).
   */
  template <typename MeshHandleT, typename SurfaceMaterialHandleT, typename TextureHandleT>
  struct LoadResult {
    std::vector<TextureHandleT> texture_handles;                   // Stage 1: Deduplicated textures
    std::vector<SurfaceMaterialHandleT> surface_material_handles;  // Stage 1: Materials with texture indices
    std::vector<MeshHandleT> mesh_handles;                         // Stage 2: Meshes
    Node root_node;                                                // Stage 3: Scene hierarchy

    bool success = false;
    std::string error_message;
  };

  /**
   * @brief Load a 3D model file
   *
   * @tparam VertexType User-defined vertex structure
   * @tparam MeshHandleT User-defined mesh handle type
   * @tparam SurfaceMaterialHandleT User-defined surface material handle type
   * @tparam TextureHandleT User-defined texture handle type
   *
   * @param file_path Path to model file (fbx, gltf, obj, etc.)
   * @param texture_callback Stage 1: Called for each unique texture
   * @param surface_material_callback Stage 1: Called for each material
   * @param mesh_callback Stage 2: Called for each mesh
   * @param options Loading options
   *
   * @return LoadResult containing all handles and scene hierarchy
   */
  template <typename VertexType, typename MeshHandleT, typename SurfaceMaterialHandleT, typename TextureHandleT>
  static LoadResult<MeshHandleT, SurfaceMaterialHandleT, TextureHandleT> Load(const std::string& file_path,
    TextureCallback texture_callback,
    SurfaceMaterialCallback surface_material_callback,
    std::function<MeshHandleT(const MeshData<VertexType>&)> mesh_callback,
    const LoadOptions& options = LoadOptions()) {
    LoadResult<MeshHandleT, SurfaceMaterialHandleT, TextureHandleT> result;

    Assimp::Importer importer;

    unsigned int import_flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_OptimizeMeshes;

    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    if (options.generate_smooth_normals) {
      import_flags |= aiProcess_GenSmoothNormals;
      importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, options.normal_smoothing_angle);
    }

    // Load the scene
    const aiScene* scene = importer.ReadFile(file_path, import_flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      result.success = false;
      result.error_message = std::string("Assimp: ") + importer.GetErrorString();
      return result;
    }

    float final_scale = options.global_scale;

    // consider the fbx unit scale
    if (scene->mMetaData) {
      float unit_scale_factor = 1.0f;
      if (scene->mMetaData->Get("UnitScaleFactor", unit_scale_factor)) {
        final_scale *= unit_scale_factor;
      }
    }

    struct EmbeddedTextureMap {
      std::unordered_map<std::string, int> exact_match;
      std::unordered_map<std::string, int> basename_match;
      std::unordered_map<std::string, int> lowercase_match;

      void Build(const aiScene* scene) {
        for (unsigned int i = 0; i < scene->mNumTextures; ++i) {
          const aiTexture* ai_tex = scene->mTextures[i];
          std::string filename = ai_tex->mFilename.C_Str();

          if (filename.empty()) {
            filename = "*" + std::to_string(i);
          }

          if (!exact_match.count(filename)) {
            exact_match[filename] = i;
          }

          std::string basename = filename;
          size_t slash_pos = basename.find_last_of("/\\");
          if (slash_pos != std::string::npos) {
            basename = basename.substr(slash_pos + 1);
          }
          if (!basename_match.count(basename)) {
            basename_match[basename] = i;
          }

          std::string lowercase = basename;
          std::transform(
            lowercase.begin(), lowercase.end(), lowercase.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
          if (!lowercase_match.count(lowercase)) {
            lowercase_match[lowercase] = i;
          }
        }
      }

      int FindIndex(const std::string& reference_path) const {
        auto it = exact_match.find(reference_path);
        if (it != exact_match.end()) return it->second;

        std::string basename = reference_path;
        size_t slash_pos = basename.find_last_of("/\\");
        if (slash_pos != std::string::npos) {
          basename = basename.substr(slash_pos + 1);
        }
        it = basename_match.find(basename);
        if (it != basename_match.end()) return it->second;

        std::string lowercase = basename;
        std::transform(
          lowercase.begin(), lowercase.end(), lowercase.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        it = lowercase_match.find(lowercase);
        if (it != lowercase_match.end()) return it->second;

        return -1;
      }
    };

    EmbeddedTextureMap embedded_map;
    embedded_map.Build(scene);

    std::unordered_map<TextureKey, uint32_t, TextureKeyHash> texture_path_to_index;
    std::vector<TextureRef> unique_textures;

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
      aiMaterial* ai_mat = scene->mMaterials[i];

      auto collect_texture = [&](aiTextureType ai_type, TextureType tex_type) {
        aiString ai_path;
        if (ai_mat->GetTexture(ai_type, 0, &ai_path) == AI_SUCCESS) {
          TextureKey key{ai_path.C_Str(), tex_type};
          if (texture_path_to_index.find(key) == texture_path_to_index.end()) {
            TextureRef tex_ref;
            tex_ref.path = ai_path.C_Str();
            tex_ref.type = tex_type;

            // Extract embedded texture data
            // First, check for direct embedded reference (*0, *1, etc.)
            if (!tex_ref.path.empty() && tex_ref.path[0] == '*') {
              tex_ref.embedded = ExtractEmbeddedTexture(scene, tex_ref.path);
            } else {
              // Check if the external path matches an embedded texture by filename
              int embedded_idx = embedded_map.FindIndex(tex_ref.path);
              if (embedded_idx >= 0) {
                std::string embedded_ref = "*" + std::to_string(embedded_idx);
                tex_ref.embedded = ExtractEmbeddedTexture(scene, embedded_ref);
              }
            }

            unique_textures.push_back(tex_ref);
            texture_path_to_index[key] = static_cast<uint32_t>(unique_textures.size() - 1);
          }
        }
      };

      collect_texture(aiTextureType_BASE_COLOR, TextureType::Color);
      collect_texture(aiTextureType_DIFFUSE, TextureType::Color);
      collect_texture(aiTextureType_NORMALS, TextureType::Data);
      collect_texture(aiTextureType_UNKNOWN, TextureType::Data);  // PBR metallic+roughness
      collect_texture(aiTextureType_EMISSIVE, TextureType::Color);
      collect_texture(aiTextureType_AMBIENT_OCCLUSION, TextureType::Data);
    }

    // Texture Callback
    for (const auto& tex_ref : unique_textures) {
      TextureHandle handle = texture_callback(tex_ref);
      result.texture_handles.push_back(handle);
    }

    // Material Callback
    std::vector<SurfaceMaterialData> materials;
    materials.reserve(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
      SurfaceMaterialData mat_data = ConvertMaterialToSurfaceData(scene->mMaterials[i], texture_path_to_index);
      materials.push_back(mat_data);
      result.surface_material_handles.push_back(surface_material_callback(mat_data));
    }

    // Mesh Callback
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
      MeshData<VertexType> mesh_data = ConvertMesh<VertexType>(scene->mMeshes[i], final_scale);
      result.mesh_handles.push_back(mesh_callback(mesh_data));
    }

    // Scene Hierarchy
    result.root_node = BuildNodeHierarchy(scene->mRootNode);

    result.success = true;
    return result;
  }

  /**
   * @brief Load model with bone and animation support (reserved for future)
   */
  template <typename VertexType, typename MeshHandleT, typename SurfaceMaterialHandleT, typename TextureHandleT>
  static LoadResult<MeshHandleT, SurfaceMaterialHandleT, TextureHandleT> LoadWithAnimation(const std::string& file_path,
    TextureCallback texture_callback,
    SurfaceMaterialCallback surface_material_callback,
    std::function<MeshHandleT(const MeshData<VertexType>&)> mesh_callback,
    BoneCallback bone_callback,
    AnimationCallback animation_callback,
    const LoadOptions& options = LoadOptions()) {
    // For now, just call Load without animation support
    // TODO: Implement bone and animation extraction
    (void)bone_callback;
    (void)animation_callback;
    return Load<VertexType, MeshHandleT, SurfaceMaterialHandleT, TextureHandleT>(
      file_path, texture_callback, surface_material_callback, mesh_callback, options);
  }

 private:
  /**
   * @brief Convert Assimp material to SurfaceMaterialData with texture indices
   *
   * @param ai_material Assimp material
   * @param texture_path_to_index Map from texture path to index in unique_textures
   * @return SurfaceMaterialData with texture handle indices
   */
  static SurfaceMaterialData ConvertMaterialToSurfaceData(
    aiMaterial* ai_material, const std::unordered_map<TextureKey, uint32_t, TextureKeyHash>& texture_path_to_index) {
    SurfaceMaterialData material;

    // Get material name
    aiString name;
    if (ai_material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
      material.name = name.C_Str();
    }

    // Get base color factor
    aiColor4D base_color(1.0f, 1.0f, 1.0f, 1.0f);
    if (ai_material->Get(AI_MATKEY_BASE_COLOR, base_color) != AI_SUCCESS) {
      ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);
    }
    material.base_color_factor = DirectX::XMFLOAT4(base_color.r, base_color.g, base_color.b, base_color.a);

    // Get metallic factor
    float metallic = 0.0f;
    if (ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
      material.metallic_factor = metallic;
    }

    // Get roughness factor
    float roughness = 0.5f;
    if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
      material.roughness_factor = roughness;
    }

    // Get emissive color
    aiColor3D emissive(0.0f, 0.0f, 0.0f);
    if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
      material.emissive_factor = DirectX::XMFLOAT3(emissive.r, emissive.g, emissive.b);
    }

    // Get alpha cutoff
    float opacity = 1.0f;
    ai_material->Get(AI_MATKEY_OPACITY, opacity);
    if (opacity < 1.0f) {
      material.alpha_cutoff = opacity;
      material.is_alpha_blend = true;
    }

    // Get double-sided flag
    int two_sided = 0;
    if (ai_material->Get(AI_MATKEY_TWOSIDED, two_sided) == AI_SUCCESS) {
      material.is_double_sided = (two_sided != 0);
    }

    // Convert texture references to texture indices
    auto get_texture_index = [&](aiTextureType ai_type, TextureType tex_type) -> std::optional<uint32_t> {
      aiString ai_path;
      if (ai_material->GetTexture(ai_type, 0, &ai_path) != AI_SUCCESS) {
        return std::nullopt;
      }
      TextureKey key{ai_path.C_Str(), tex_type};
      auto it = texture_path_to_index.find(key);
      if (it != texture_path_to_index.end()) {
        return it->second;
      }
      return std::nullopt;
    };

    material.base_color_texture_index = get_texture_index(aiTextureType_BASE_COLOR, TextureType::Color);
    if (!material.base_color_texture_index.has_value()) {
      material.base_color_texture_index = get_texture_index(aiTextureType_DIFFUSE, TextureType::Color);
    }

    material.normal_texture_index = get_texture_index(aiTextureType_NORMALS, TextureType::Data);
    material.metallic_roughness_texture_index = get_texture_index(aiTextureType_UNKNOWN, TextureType::Data);
    material.emissive_texture_index = get_texture_index(aiTextureType_EMISSIVE, TextureType::Color);
    material.occlusion_texture_index = get_texture_index(aiTextureType_AMBIENT_OCCLUSION, TextureType::Data);

    return material;
  }

  /**
   * @brief Convert Assimp mesh to MeshData with user-defined vertex type
   *
   * Requires VertexType to have a 'position' member (float[3] or XMFLOAT3).
   * Optional members: normal, tangent, texcoord (detected at compile-time).
   */
  template <typename VertexType>
  static MeshData<VertexType> ConvertMesh(aiMesh* ai_mesh, float global_scale) {
    MeshData<VertexType> mesh_data;
    mesh_data.name = ai_mesh->mName.C_Str();
    mesh_data.material_index = ai_mesh->mMaterialIndex;

    mesh_data.vertices.reserve(ai_mesh->mNumVertices);
    mesh_data.indices.reserve(ai_mesh->mNumFaces * 3);

    // Convert vertices
    for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
      VertexType vertex{};

      // Position (required)
      if constexpr (VertexWithPosition<VertexType>) {
        if constexpr (requires { vertex.position[0]; }) {
          // C array style: float position[3]
          vertex.position[0] = ai_mesh->mVertices[i].x * global_scale;
          vertex.position[1] = ai_mesh->mVertices[i].y * global_scale;
          vertex.position[2] = ai_mesh->mVertices[i].z * global_scale;
        } else {
          // XMFLOAT3 style
          vertex.position = DirectX::XMFLOAT3(
            ai_mesh->mVertices[i].x * global_scale, ai_mesh->mVertices[i].y * global_scale, ai_mesh->mVertices[i].z * global_scale);
        }
      } else {
        static_assert(sizeof(VertexType) == 0, "VertexType must have a 'position' member (float[3] or XMFLOAT3)");
      }

      // Normal (optional)
      if constexpr (VertexWithNormal<VertexType>) {
        if (ai_mesh->HasNormals()) {
          if constexpr (requires { vertex.normal[0]; }) {
            vertex.normal[0] = ai_mesh->mNormals[i].x;
            vertex.normal[1] = ai_mesh->mNormals[i].y;
            vertex.normal[2] = ai_mesh->mNormals[i].z;
          } else {
            vertex.normal = DirectX::XMFLOAT3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
          }
        }
      }

      // Tangent (optional)
      if constexpr (VertexWithTangent<VertexType>) {
        if (ai_mesh->HasTangentsAndBitangents()) {
          if constexpr (requires { vertex.tangent[0]; }) {
            vertex.tangent[0] = ai_mesh->mTangents[i].x;
            vertex.tangent[1] = ai_mesh->mTangents[i].y;
            vertex.tangent[2] = ai_mesh->mTangents[i].z;
            if constexpr (requires { vertex.tangent[3]; }) {
              vertex.tangent[3] = 1.0f;
            }
          } else {
            vertex.tangent = DirectX::XMFLOAT4(ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z, 1.0f);
          }
        }
      }

      // Texcoord (optional)
      if constexpr (VertexWithTexcoord<VertexType>) {
        if (ai_mesh->HasTextureCoords(0)) {
          float u = ai_mesh->mTextureCoords[0][i].x;
          float v = ai_mesh->mTextureCoords[0][i].y;

          if constexpr (requires { vertex.texcoord[0]; }) {
            vertex.texcoord[0] = u;
            vertex.texcoord[1] = v;
          } else {
            vertex.texcoord = DirectX::XMFLOAT2(u, v);
          }
        }
      }

      // Color (optional) - Initialize to white (1,1,1,1) so it doesn't darken the texture
      if constexpr (requires { vertex.color[0]; }) {
        // C array style: float color[4]
        vertex.color[0] = 1.0f;
        vertex.color[1] = 1.0f;
        vertex.color[2] = 1.0f;
        vertex.color[3] = 1.0f;
      } else if constexpr (requires { vertex.color; }) {
        // XMFLOAT4 style
        vertex.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
      }

      mesh_data.vertices.push_back(vertex);
    }

    // Convert indices
    for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
      aiFace& face = ai_mesh->mFaces[i];
      if (face.mNumIndices == 3) {
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
          mesh_data.indices.push_back(face.mIndices[j]);
        }
      }
    }

    return mesh_data;
  }

  /**
   * @brief Extract embedded texture data from Assimp scene
   *
   * Embedded textures have paths like "*0", "*1", etc.
   * Returns nullopt if the texture index is invalid.
   *
   * @param scene Assimp scene containing embedded textures
   * @param path Texture path (e.g., "*0" for first embedded texture)
   * @return EmbeddedTexture data if successful, nullopt otherwise
   */
  static std::optional<EmbeddedTexture> ExtractEmbeddedTexture(const aiScene* scene, const std::string& path) {
    if (path.size() < 2 || path[0] != '*') {
      return std::nullopt;
    }

    int index = 0;
    try {
      index = std::stoi(path.substr(1));
    } catch (const std::exception&) {
      return std::nullopt;
    }

    if (index < 0 || index >= static_cast<int>(scene->mNumTextures)) {
      return std::nullopt;
    }

    const aiTexture* ai_tex = scene->mTextures[index];
    EmbeddedTexture embedded;

    if (ai_tex->mHeight == 0) {
      // Compressed format (PNG, JPG, etc.)
      embedded.is_compressed = true;
      embedded.format_hint = ai_tex->achFormatHint;
      embedded.width = ai_tex->mWidth;
      embedded.height = 0;
      embedded.data.resize(ai_tex->mWidth);
      std::memcpy(embedded.data.data(), ai_tex->pcData, ai_tex->mWidth);
    } else {
      // Uncompressed RGBA8888
      embedded.is_compressed = false;
      embedded.format_hint = "rgba8888";
      embedded.width = ai_tex->mWidth;
      embedded.height = ai_tex->mHeight;
      size_t pixel_count = embedded.width * embedded.height;
      embedded.data.resize(pixel_count * 4);

      for (size_t i = 0; i < pixel_count; ++i) {
        const aiTexel& texel = ai_tex->pcData[i];
        embedded.data[i * 4 + 0] = texel.r;
        embedded.data[i * 4 + 1] = texel.g;
        embedded.data[i * 4 + 2] = texel.b;
        embedded.data[i * 4 + 3] = texel.a;
      }
    }

    return embedded;
  }

  /**
   * @brief Build scene node hierarchy from Assimp node
   */
  static Node BuildNodeHierarchy(aiNode* ai_node) {
    Node node;
    node.name = ai_node->mName.C_Str();

    // Convert transform
    aiVector3D ai_scale;
    aiQuaternion ai_rotation;
    aiVector3D ai_position;
    ai_node->mTransformation.Decompose(ai_scale, ai_rotation, ai_position);

    node.transform.translation = DirectX::XMFLOAT3(ai_position.x, ai_position.y, ai_position.z);
    node.transform.rotation = DirectX::XMFLOAT4(ai_rotation.x, ai_rotation.y, ai_rotation.z, ai_rotation.w);
    node.transform.scale = DirectX::XMFLOAT3(ai_scale.x, ai_scale.y, ai_scale.z);

    static_assert(sizeof(DirectX::XMFLOAT4X4) == sizeof(aiMatrix4x4));
    // BUG: needs transpose for DirectX row-major convention, not used currently
    std::memcpy(&node.transform.local_matrix, &ai_node->mTransformation, sizeof(DirectX::XMFLOAT4X4));

    // Store mesh indices
    node.mesh_indices.reserve(ai_node->mNumMeshes);
    for (unsigned int i = 0; i < ai_node->mNumMeshes; ++i) {
      node.mesh_indices.push_back(ai_node->mMeshes[i]);
    }

    // Recursively build children
    node.children.reserve(ai_node->mNumChildren);
    for (unsigned int i = 0; i < ai_node->mNumChildren; ++i) {
      node.children.push_back(BuildNodeHierarchy(ai_node->mChildren[i]));
    }

    return node;
  }
};

}  // namespace Model
