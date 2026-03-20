#include "render_service.h"

#include "graphic.h"

MaterialHandle RenderService::AllocateMaterial(const MaterialDescriptor& descriptor) {
  return graphic_.GetMaterialDescriptorPool().Allocate(descriptor);
}

void RenderService::UpdateMaterial(MaterialHandle handle, const MaterialDescriptor& descriptor) {
  graphic_.GetMaterialDescriptorPool().Update(handle, descriptor);
}

void RenderService::FreeMaterial(MaterialHandle handle) {
  graphic_.GetMaterialDescriptorPool().Free(handle);
}

uint32_t RenderService::GetSceneWidth() const {
  return graphic_.GetSceneWidth();
}

uint32_t RenderService::GetSceneHeight() const {
  return graphic_.GetSceneHeight();
}

uint32_t RenderService::GetNormalDepthSrvIndex() const {
  return graphic_.GetNormalDepthSrvIndex();
}

uint32_t RenderService::GetUIBlurSrvIndex() const {
  return graphic_.GetUIBlurSrvIndex();
}

void RenderService::SetChromaticAberrationIntensity(float intensity) {
  graphic_.GetChromaticAberrationConfig().intensity = intensity;
}

float RenderService::GetChromaticAberrationIntensity() const {
  return graphic_.GetChromaticAberrationConfig().intensity;
}

void RenderService::WaitForGpuIdle() {
  graphic_.WaitForGpuIdle();
}
