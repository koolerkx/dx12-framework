/**
 * @file render_service.h
 * @brief Abstract service interfaces for Game layer to access Graphic functionality.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "Framework/Math/Math.h"
#include "Framework/Render/material_descriptor.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Shader/shader_id.h"

class IShaderNameService {
 public:
  virtual ~IShaderNameService() = default;
  virtual std::string_view GetName(ShaderId shader_id) const = 0;
  virtual std::optional<ShaderId> FindIdByName(std::string_view name) const = 0;
};

class IDebugDrawService {
 public:
  virtual ~IDebugDrawService() = default;
  virtual void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) = 0;
};

class IRenderService {
 public:
  virtual ~IRenderService() = default;

  virtual MaterialHandle AllocateMaterial(const MaterialDescriptor& descriptor) = 0;
  virtual MaterialHandle UpdateMaterial(MaterialHandle handle, const MaterialDescriptor& descriptor) = 0;
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
