/**
 * @file render_service.h
 * @brief Abstract render service interface for Game layer to access Graphic functionality.
 */
#pragma once

#include <cstdint>

#include "Framework/Render/material_descriptor.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/shader_name_service.h"

class IRenderService {
 public:
  virtual ~IRenderService() = default;

  virtual MaterialHandle AllocateMaterial(const MaterialDescriptor& descriptor) = 0;
  virtual void UpdateMaterial(MaterialHandle handle, const MaterialDescriptor& descriptor) = 0;
  virtual void FreeMaterial(MaterialHandle handle) = 0;

  virtual uint32_t GetSceneWidth() const = 0;
  virtual uint32_t GetSceneHeight() const = 0;
  virtual uint32_t GetNormalDepthSrvIndex() const = 0;
  virtual uint32_t GetUIBlurSrvIndex() const = 0;

  virtual void SetChromaticAberrationIntensity(float intensity) = 0;
  virtual float GetChromaticAberrationIntensity() const = 0;

  virtual void WaitForGpuIdle() = 0;

  virtual IShaderNameService& GetShaderNameService() = 0;
};
