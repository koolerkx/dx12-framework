#include "asset_manager.h"

#include "Framework/Logging/logger.h"
#include "Graphic/Resource/Font/sprite_font_manager.h"
#include "Graphic/Resource/Texture/texture_manager.h"
#include "Graphic/Resource/mesh_factory.h"
#include "Graphic/graphic.h"
#include "text_mesh_handle.h"

class AssetManager::Impl {
 public:
  Graphic* graphic = nullptr;
  TextureManager* texture_manager = nullptr;
  Font::SpriteFontManager* font_manager = nullptr;
  // Todo: ModelManager* model_manager = nullptr;
  // Todo: AudioManager* audio_manager = nullptr;
};

// must defined in cpp
AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;
AssetManager::AssetManager(AssetManager&&) noexcept = default;
AssetManager& AssetManager::operator=(AssetManager&&) noexcept = default;

bool AssetManager::Initialize(Graphic* graphic) {
  impl_ = std::make_unique<Impl>();
  impl_->graphic = graphic;
  impl_->texture_manager = &graphic->GetTextureManager();
  impl_->font_manager = &graphic->GetSpriteFontManager();
  CreateDefaultMeshes();
  return true;
}

void AssetManager::Shutdown() {
  impl_.reset();
}

AssetHandle<Texture> AssetManager::LoadTexture(const std::string& path) {
  auto texture = impl_->texture_manager->LoadTexture(path);
  return AssetHandle<Texture>(texture, path);
}

AssetHandle<Texture> AssetManager::LoadCubemap(const std::string& path) {
  auto texture = impl_->texture_manager->LoadCubemapFromCrossHDR(path);
  return AssetHandle<Texture>(texture, path);
}

std::vector<AssetHandle<Texture>> AssetManager::LoadTextures(const std::vector<std::string>& paths) {
  auto textures = impl_->texture_manager->LoadTextures(paths);

  std::vector<AssetHandle<Texture>> handles;
  handles.reserve(textures.size());
  for (size_t i = 0; i < textures.size(); ++i) {
    handles.emplace_back(textures[i], paths[i]);
  }
  return handles;
}

void AssetManager::ProcessDeferredCleanup(uint64_t completed_fence_value) {
  impl_->texture_manager->ProcessDeferredFrees(completed_fence_value);
}

void AssetManager::ClearUploadBuffers() {
  impl_->texture_manager->CleanUploadBuffers();
}

Graphic* AssetManager::GetGraphicForDebugUseOnly() const {
  return impl_.get()->graphic;
}

void AssetManager::CreateDefaultMeshes() {
  if (!impl_->graphic) return;

  ID3D12Device* device = impl_->graphic->GetDevice();

  // Create and own Quad mesh
  auto quad_mesh = std::make_unique<Mesh>();
  if (MeshFactory::CreateQuad(device, *quad_mesh)) {
    default_meshes_[DefaultMesh::Quad] = quad_mesh.get();
    owned_default_meshes_[DefaultMesh::Quad] = std::move(quad_mesh);
  }

  // Create and own Cube mesh
  auto cube_mesh = std::make_unique<Mesh>();
  if (MeshFactory::CreateCube(device, *cube_mesh)) {
    default_meshes_[DefaultMesh::Cube] = cube_mesh.get();
    owned_default_meshes_[DefaultMesh::Cube] = std::move(cube_mesh);
  }

  auto rect_mesh = std::make_unique<Mesh>();
  if (MeshFactory::CreateRect(device, *rect_mesh)) {
    default_meshes_[DefaultMesh::Rect] = rect_mesh.get();
    owned_default_meshes_[DefaultMesh::Rect] = std::move(rect_mesh);
  }

  // Create and own Plane mesh (10x10 subdivisions for terrain)
  auto plane_mesh = std::make_unique<Mesh>();
  if (MeshFactory::CreatePlane(device, *plane_mesh, 10, 10)) {
    default_meshes_[DefaultMesh::Plane] = plane_mesh.get();
    owned_default_meshes_[DefaultMesh::Plane] = std::move(plane_mesh);
  }
}

const Mesh* AssetManager::GetDefaultMesh(DefaultMesh type) const {
  auto it = default_meshes_.find(type);
  if (it != default_meshes_.end()) {
    return it->second;
  }

  Logger::LogFormat(
    LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Default mesh not found: {}", static_cast<int>(type));
  return nullptr;
}

bool AssetManager::LoadFont(Font::FontFamily family, const std::string& fnt_path, const std::string& texture_path) {
  if (!impl_->font_manager) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font manager not initialized");
    return false;
  }

  return impl_->font_manager->LoadFontVariant(family, fnt_path, texture_path);
}

TextMeshHandle AssetManager::CreateTextMesh(
  const std::wstring& text, Font::FontFamily family, float pixel_size, const Text::TextLayoutProps& layout_props) {
  if (!impl_->font_manager) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font manager not initialized");
    return TextMeshHandle();
  }

  // Create text layout - pure CPU data
  Font::TextLayoutData layout_data;
  if (!impl_->font_manager->CreateTextLayout(text, family, pixel_size, layout_props, layout_data)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to create text layout");
    return TextMeshHandle();
  }

  // Convert to GlyphLayoutData with UV transform data
  std::vector<GlyphLayoutData> glyphs;
  glyphs.reserve(layout_data.glyphs.size());

  for (const auto& glyph : layout_data.glyphs) {
    GlyphLayoutData glyph_data;
    glyph_data.x = glyph.x;
    glyph_data.y = glyph.y;
    glyph_data.width = glyph.width;
    glyph_data.height = glyph.height;

    glyph_data.uv_offset = {glyph.u0, glyph.v1};
    glyph_data.uv_scale = {glyph.u1 - glyph.u0, glyph.v0 - glyph.v1};
    glyphs.push_back(glyph_data);
  }

  // Get texture from layout (non-owning pointer)
  Texture* texture = layout_data.variant ? layout_data.variant->texture.get() : nullptr;

  // Return handle with pure CPU data - no mesh ownership
  // TextRenderer will use shared Quad mesh from AssetManager
  return TextMeshHandle(std::move(glyphs), layout_data.width, layout_data.height, texture);
}
