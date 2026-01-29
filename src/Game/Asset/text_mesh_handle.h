#pragma once
#include <DirectXMath.h>

#include <vector>

struct Texture;

namespace Font {
enum class FontFamily : uint16_t;
}  // namespace Font

// Glyph layout data
struct GlyphLayoutData {
  float x = 0.0f;                              // Position X in layout space (pixels)
  float y = 0.0f;                              // Position Y in layout space (pixels)
  float width = 0.0f;                          // Glyph width (pixels)
  float height = 0.0f;                         // Glyph height (pixels)
  DirectX::XMFLOAT2 uv_offset = {0.0f, 0.0f};  // UV offset in atlas
  DirectX::XMFLOAT2 uv_scale = {1.0f, 1.0f};   // UV scale in atlas
};

// Text layout handle - pure CPU data structure
class TextMeshHandle {
 public:
  TextMeshHandle() = default;

  // Construct from layout data
  TextMeshHandle(std::vector<GlyphLayoutData> glyphs, float width, float height, Texture* texture)
      : glyphs_(std::move(glyphs)), texture_(texture), width_(width), height_(height) {};

  // Default move/copy - it's just CPU data
  TextMeshHandle(TextMeshHandle&&) noexcept = default;
  TextMeshHandle& operator=(TextMeshHandle&&) noexcept = default;
  TextMeshHandle(const TextMeshHandle&) = default;
  TextMeshHandle& operator=(const TextMeshHandle&) = default;

  // Get number of glyphs
  size_t GetGlyphCount() const {
    return glyphs_.size();
  }

  // Get glyph layout data by index
  const GlyphLayoutData* GetGlyph(size_t index) const {
    if (index >= glyphs_.size()) {
      return nullptr;
    }
    return &glyphs_[index];
  }

  // Get texture (font atlas)
  Texture* GetTexture() const {
    return texture_;
  }

  // Get total text bounding box size
  float GetWidth() const {
    return width_;
  }
  float GetHeight() const {
    return height_;
  }

  // Check if valid
  bool IsValid() const {
    return !glyphs_.empty() && texture_ != nullptr;
  }

 private:
  std::vector<GlyphLayoutData> glyphs_;
  Texture* texture_ = nullptr;  // Non-owning pointer to font atlas
  float width_ = 0.0f;
  float height_ = 0.0f;
};
