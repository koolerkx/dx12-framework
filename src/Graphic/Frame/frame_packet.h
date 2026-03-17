#pragma once
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "Framework/Core/color.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/instance_data.h"
#include "Framework/Render/render_request.h"
#include "Graphic/Render/shadow_config.h"
#include "camera_data.h"
#include "draw_command.h"

static constexpr uint32_t MAX_POINT_LIGHTS = 8;

enum class BackgroundMode : uint8_t { ClearColor, Skybox };

struct BackgroundConfig {
  BackgroundMode mode = BackgroundMode::ClearColor;
  Color clear_color = colors::DarkSlateGray;
  uint32_t cubemap_srv_index = UINT32_MAX;
};

struct LightingConfig {
  Math::Vector3 direction = Math::Vector3(0.0f, -1.0f, 0.0f);
  float intensity = 1.0f;
  Math::Vector3 directional_color = Math::Vector3(1.0f, 1.0f, 1.0f);
  float ambient_intensity = 0.2f;
  Math::Vector3 ambient_color = Math::Vector3(1.0f, 1.0f, 1.0f);
};

enum class ShadowAlgorithm : uint32_t {
  Hard = 0,
  PCF3x3 = 1,
  PoissonDisk = 2,
  RotatedPoissonDisk = 3,
  PCSS = 4,
};

struct ShadowConfig {
  uint32_t resolution = 2048;
  uint32_t cascade_count = ShadowCascadeConfig::DEFAULT_CASCADE_COUNT;
  float cascade_depth_bias[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS};
  float cascade_normal_bias[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS};
  float shadow_distance = 100.0f;
  float light_distance = 100.0f;
  float cascade_blend_range = 0.1f;
  ShadowAlgorithm algorithm = ShadowAlgorithm::PCF3x3;
  Math::Vector3 shadow_color = Math::Vector3(0.0f, 0.0f, 0.0f);
  float light_size = 1.0f;
  bool enabled = true;
};

struct PointLightEntry {
  Math::Vector3 position;
  float intensity = 1.0f;
  Math::Vector3 color = {1.0f, 1.0f, 1.0f};
  float radius = 10.0f;
  float falloff = 2.0f;
};

struct InstanceDataRef {
  uint32_t offset = 0;
  uint32_t size = 0;
  uint32_t count = 0;
  bool IsValid() const { return size > 0; }
};

struct InternalInstancedRequest {
  InstancedRenderRequest request;
  InstanceDataRef instance_data;
};

struct FramePacket {
  float time = 0.0f;
  CameraData main_camera;
  CameraData ui_camera;
  BackgroundConfig background;
  LightingConfig lighting;
  ShadowConfig shadow;
  std::vector<DrawCommand> commands;
  std::vector<PointLightEntry> point_lights;

  std::vector<RenderRequest> single_requests;
  std::vector<InternalInstancedRequest> instanced_requests;
  std::vector<std::byte> instance_data_pool;

  void Clear() {
    commands.clear();
    point_lights.clear();
    single_requests.clear();
    instanced_requests.clear();
    instance_data_pool.clear();
  }

  void AddCommand(DrawCommand cmd) {
    commands.emplace_back(std::move(cmd));
  }

  void Draw(RenderRequest request) {
    single_requests.emplace_back(std::move(request));
  }

  void DrawInstanced(InstancedRenderRequest request, std::span<const InstanceData> instances) {
    uint32_t offset = static_cast<uint32_t>(instance_data_pool.size());
    uint32_t byte_size = static_cast<uint32_t>(instances.size_bytes());
    instance_data_pool.resize(instance_data_pool.size() + byte_size);
    std::memcpy(instance_data_pool.data() + offset, instances.data(), byte_size);

    instanced_requests.push_back({
      .request = std::move(request),
      .instance_data = {offset, byte_size, static_cast<uint32_t>(instances.size())},
    });
  }

  void AddPointLight(PointLightEntry entry) {
    if (point_lights.size() < MAX_POINT_LIGHTS) point_lights.emplace_back(std::move(entry));
  }
};
