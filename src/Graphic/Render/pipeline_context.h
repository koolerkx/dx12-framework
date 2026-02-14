#pragma once

#include <cstdint>

struct ID3D12Device;
class ShaderManager;

struct PipelineContext {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  uint32_t width;
  uint32_t height;
};
