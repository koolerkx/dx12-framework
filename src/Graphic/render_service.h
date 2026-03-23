/**
 * @file render_service.h
 * @brief Concrete IRenderService implementation that delegates to Graphic subsystems.
 */
#pragma once

#include "Framework/Render/render_service.h"
#include "Framework/Render/shader_name_service.h"
#include "Graphic/Pipeline/shader_registry.h"

class Graphic;

class ShaderNameService final : public IShaderNameService {
 public:
  std::string_view GetName(ShaderId shader_id) const override {
    return ShaderRegistry::GetName(shader_id);
  }

  std::optional<ShaderId> FindIdByName(std::string_view name) const override {
    return ShaderRegistry::FindIdByName(name);
  }
};

class RenderService final : public IRenderService {
 public:
  explicit RenderService(Graphic& graphic) : graphic_(graphic) {
  }

  MaterialHandle AllocateMaterial(const MaterialDescriptor& descriptor) override;
  void UpdateMaterial(MaterialHandle handle, const MaterialDescriptor& descriptor) override;
  void FreeMaterial(MaterialHandle handle) override;

  uint32_t GetSceneWidth() const override;
  uint32_t GetSceneHeight() const override;
  uint32_t GetNormalDepthSrvIndex() const override;
  uint32_t GetUIBlurSrvIndex() const override;

  void SetChromaticAberrationIntensity(float intensity) override;
  float GetChromaticAberrationIntensity() const override;

  void WaitForGpuIdle() override;

  IShaderNameService& GetShaderNameService() override {
    return shader_name_service_;
  }

 private:
  Graphic& graphic_;
  ShaderNameService shader_name_service_;
};
