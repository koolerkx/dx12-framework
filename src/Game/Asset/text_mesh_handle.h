#pragma once
#include <vector>

#include "Framework/Math/Math.h"
#include "Framework/Render/texture_handle.h"

using Math::Vector2;

namespace Font {
enum class FontFamily : uint16_t;
}  // namespace Font

struct GlyphLayoutData {
  float x = 0.0f;                    // Position X in layout space (pixels)
  float y = 0.0f;                    // Position Y in layout space (pixels)
  float width = 0.0f;                // Glyph width (pixels)
  float height = 0.0f;               // Glyph height (pixels)
  Vector2 uv_offset = {0.0f, 0.0f};  // UV offset in atlas
  Vector2 uv_scale = {1.0f, 1.0f};   // UV scale in atlas
};

// Text layout handle - pure CPU data structure
class TextMeshHandle {
 public:
  TextMeshHandle() = default;

  TextMeshHandle(std::vector<GlyphLayoutData> glyphs, float width, float height, TextureHandle texture)
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

  TextureHandle GetTexture() const {
    return texture_;
  }

  // Get total text bounding box size
  float GetWidth() const {
    return width_;
  }
  float GetHeight() const {
    return height_;
  }

  bool IsValid() const {
    return !glyphs_.empty() && texture_.IsValid();
  }

 private:
  std::vector<GlyphLayoutData> glyphs_;
  TextureHandle texture_;
  float width_ = 0.0f;
  float height_ = 0.0f;
};
