#include "particle_emitter.h"

#include <cmath>
#if ENABLE_EDITOR
#include "Framework/Editor/editor_ui.h"
#endif

#include "Framework/Asset/asset_manager.h"
#include "Framework/Shader/shader_descriptor.h"
#include "Framework/Shader/shader_id.h"
#include "Framework/Shader/shader_registration.h"
#include "Framework/Shader/vertex_shaders.h"
#include "SoftParticleCB.generated.h"
#include "game_context.h"

thread_local std::mt19937 ParticleEmitter::rng_{std::random_device{}()};

ParticleEmitter::ParticleEmitter(GameObject* owner) : RendererComponent(owner), spawn_fn_(SpawnFromCenter()) {
}

ParticleEmitter::ParticleEmitter(GameObject* owner, const Props& props) : RendererComponent(owner) {
  if (!props.texture_path.empty()) SetTexturePath(props.texture_path);
  max_particles_ = props.max_particles;
  emit_rate_ = props.emit_rate;
  particle_lifetime_ = props.particle_lifetime;
  particle_size_ = props.particle_size;
  start_color_ = props.start_color;
  end_color_ = props.end_color;
  start_speed_ = props.start_speed;
  speed_variation_ = props.speed_variation;
  gravity_ = props.gravity;
  loop_ = props.loop;
  spawn_offset_ = props.spawn_offset;
  spawn_shape_ = props.spawn_shape;
  spawn_radius_ = props.spawn_radius;
  fade_in_ratio_ = props.fade_in_ratio;
  fade_out_ratio_ = props.fade_out_ratio;
  emissive_intensity_ = props.emissive_intensity;
  soft_distance_ = props.soft_distance;

  burst_count_ = props.burst_count;
  drag_ = props.drag;
  end_size_ = props.end_size;
  lifetime_variation_ = props.lifetime_variation;
  size_variation_ = props.size_variation;

  switch (spawn_shape_) {
    case SpawnShape::Disk:
      spawn_fn_ = SpawnFromDisk();
      break;
    case SpawnShape::Custom:
      spawn_fn_ = props.spawn_fn ? props.spawn_fn : SpawnFromCenter();
      break;
    default:
      spawn_fn_ = SpawnFromCenter();
      break;
  }

  render_settings_ = Rendering::RenderSettings::Transparent();
  render_settings_.blend_mode = props.blend_mode;
}

void ParticleEmitter::Play() {
  is_playing_ = true;
  emit_accumulator_ = 0.0f;

  if (burst_count_ > 0) {
    Vector3 emitter_pos = GetOwner()->GetTransform()->GetWorldPosition();
    std::uniform_real_distribution<float> speed_dist(start_speed_ - speed_variation_, start_speed_ + speed_variation_);
    std::uniform_real_distribution<float> lifetime_dist(
      particle_lifetime_ * (1.0f - lifetime_variation_), particle_lifetime_ * (1.0f + lifetime_variation_));
    std::uniform_real_distribution<float> size_dist(1.0f - size_variation_, 1.0f + size_variation_);

    for (uint32_t i = 0; i < burst_count_ && particles_.size() < max_particles_; ++i) {
      auto [offset, direction] = spawn_fn_(rng_);

      Particle p;
      p.position = emitter_pos + spawn_offset_ + offset * spawn_radius_;
      p.velocity = direction * speed_dist(rng_);
      p.color = start_color_;
      p.start_size = size_dist(rng_);
      p.size = p.start_size;
      p.lifetime = lifetime_dist(rng_);
      p.age = 0.0f;
      particles_.push_back(p);
    }
    is_playing_ = false;
  }
}

void ParticleEmitter::Stop() {
  is_playing_ = false;
}

void ParticleEmitter::Clear() {
  particles_.clear();
  emit_accumulator_ = 0.0f;
}

void ParticleEmitter::OnUpdate(float dt) {
  if (is_playing_) {
    EmitParticles(dt);
  }
  UpdateParticles(dt);
  RemoveDeadParticles();

  if (!is_playing_ && particles_.empty() && on_all_dead_) {
    auto callback = std::move(on_all_dead_);
    callback();
  }
}

void ParticleEmitter::OnRender(FramePacket& packet) {
  if (!texture_.IsValid() || particles_.empty()) return;

  auto* context = GetOwner()->GetContext();
  auto* rs = context->GetRenderService();

  if (!shader_registered_) {
    if (auto* reg = context->GetShaderRegistration()) {
      ShaderDescriptor desc{
        .id = HashShaderName("SoftParticle"),
        .name = "SoftParticle",
        .vs_path = VS::Sprite::PATH,
        .ps_path = L"Content/shaders/soft_particle.ps.cso",
        .vertex_format = VS::Sprite::VERTEX_FORMAT,
      };
      reg->RegisterShader(desc);
    }
    shader_registered_ = true;
  }

  if (!material_handle_.IsValid() || material_dirty_) {
    MaterialDescriptor desc{};
    desc.albedo_texture_index = texture_.GetBindlessIndex();
    desc.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
    if (!material_handle_.IsValid()) {
      material_handle_ = rs->AllocateMaterial(desc);
    } else {
      material_handle_ = rs->UpdateMaterial(material_handle_, desc);
    }
    material_dirty_ = false;
  }

  MeshHandle rect_handle = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::Rect);
  if (!rect_handle.IsValid()) return;

  std::vector<InstanceData> particle_instances;
  particle_instances.reserve(particles_.size());
  for (const auto& p : particles_) {
    Matrix4 billboard_rot = Matrix4::FaceTo(p.position, packet.main_camera.position, Vector3::Up);
    Matrix4 scale_mat = Matrix4::CreateScale({particle_size_.x * p.size, particle_size_.y * p.size, 1.0f});
    Matrix4 world = scale_mat * billboard_rot * Matrix4::CreateTranslation(p.position);

    particle_instances.push_back({
      .world = world,
      .color = p.color,
      .uv_offset = {0, 0},
      .uv_scale = {1, 1},
      .overlay_color = {0, 0, 0, 0},
    });
  }

  InstancedRenderRequest request;
  request.mesh = rect_handle;
  request.shader_id = HashShaderName("SoftParticle");
  request.render_settings = render_settings_;
  request.material = material_handle_;
  request.depth = Vector3::DistanceSquared(GetOwner()->GetTransform()->GetWorldPosition(), packet.main_camera.position);
  request.layer = RenderLayer::Transparent;
  request.tags = 0;

  SoftParticleCB params{};
  params.depthSrvIndex = rs->GetNormalDepthSrvIndex();
  params.emissiveIntensity = emissive_intensity_;
  params.softDistance = soft_distance_;
  static_assert(sizeof(params) <= sizeof(request.custom_data.data));
  memcpy(request.custom_data.data.data(), &params, sizeof(params));
  request.custom_data.active = true;

  packet.DrawInstanced(std::move(request), particle_instances);
}

void ParticleEmitter::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    if (context && context->GetRenderService()) {
      context->GetRenderService()->FreeMaterial(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}

void ParticleEmitter::EmitParticles(float dt) {
  emit_accumulator_ += dt;
  float interval = 1.0f / emit_rate_;

  std::uniform_real_distribution<float> speed_dist(start_speed_ - speed_variation_, start_speed_ + speed_variation_);
  std::uniform_real_distribution<float> lifetime_dist(
    particle_lifetime_ * (1.0f - lifetime_variation_), particle_lifetime_ * (1.0f + lifetime_variation_));
  std::uniform_real_distribution<float> size_dist(1.0f - size_variation_, 1.0f + size_variation_);

  while (emit_accumulator_ >= interval && particles_.size() < max_particles_) {
    emit_accumulator_ -= interval;

    Vector3 emitter_pos = GetOwner()->GetTransform()->GetWorldPosition();
    auto [offset, direction] = spawn_fn_(rng_);

    Particle p;
    p.position = emitter_pos + spawn_offset_ + offset * spawn_radius_;
    p.velocity = direction * speed_dist(rng_);
    p.color = start_color_;
    p.start_size = size_dist(rng_);
    p.size = p.start_size;
    p.lifetime = lifetime_dist(rng_);
    p.age = 0.0f;
    particles_.push_back(p);
  }

  if (!loop_ && emit_accumulator_ >= interval) {
    is_playing_ = false;
  }
}

void ParticleEmitter::UpdateParticles(float dt) {
  for (auto& p : particles_) {
    p.velocity = p.velocity + gravity_ * dt;
    if (drag_ > 0.0f) {
      p.velocity = p.velocity * std::exp(-drag_ * dt);
    }
    p.position = p.position + p.velocity * dt;
    p.age += dt;

    float t = p.GetLifetimeRatio();
    p.color = Vector4::LerpClamped(start_color_, end_color_, t);

    float fade = 1.0f;
    if (fade_in_ratio_ > 0.0f && t < fade_in_ratio_) {
      fade = t / fade_in_ratio_;
    }
    if (fade_out_ratio_ > 0.0f && t > (1.0f - fade_out_ratio_)) {
      fade = (1.0f - t) / fade_out_ratio_;
    }
    p.color.w *= fade;

    p.size = std::lerp(p.start_size, end_size_ * p.start_size, t);
  }
}

void ParticleEmitter::RemoveDeadParticles() {
  std::erase_if(particles_, [](const Particle& p) { return !p.IsAlive(); });
}

ParticleEmitter::SpawnFn ParticleEmitter::SpawnFromCenter() {
  return [](std::mt19937& rng) -> SpawnParams {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    Vector3 dir;
    do {
      dir = {dist(rng), dist(rng), dist(rng)};
    } while (dir.LengthSquared() < 0.0001f || dir.LengthSquared() > 1.0f);
    return {.offset = Vector3::Zero, .direction = dir.Normalized()};
  };
}

ParticleEmitter::SpawnFn ParticleEmitter::SpawnFromDisk() {
  return [](std::mt19937& rng) -> SpawnParams {
    std::uniform_real_distribution<float> angle_dist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);

    float angle = angle_dist(rng);
    float r = std::sqrt(radius_dist(rng));

    float x = r * std::cos(angle);
    float z = r * std::sin(angle);

    return {
      .offset = {x, 0.0f, z},
      .direction = Vector3::Up,
    };
  };
}

#if ENABLE_EDITOR
void ParticleEmitter::OnInspectorGUI() {
  if (IsPlaying()) {
    if (editor_ui::Button("Stop")) Stop();
  } else {
    if (editor_ui::Button("Play")) Play();
  }
  editor_ui::SameLine();
  if (editor_ui::Button("Clear")) Clear();

  auto data = GetEditorData();

  editor_ui::DragFloat("Emit Rate", &data.emit_rate, 0.5f, 0.1f, 200.0f);
  editor_ui::DragFloat("Lifetime", &data.particle_lifetime, 0.1f, 0.1f, 30.0f);
  editor_ui::DragFloat2("Particle Size", &data.particle_size.x, 0.01f, 0.01f, 10.0f);
  editor_ui::ColorEdit4("Start Color", &data.start_color.x);
  editor_ui::ColorEdit4("End Color", &data.end_color.x);
  editor_ui::DragFloat("Start Speed", &data.start_speed, 0.1f, 0.0f, 50.0f);
  editor_ui::DragFloat("Speed Variation", &data.speed_variation, 0.1f, 0.0f, 50.0f);
  editor_ui::DragFloat3("Gravity", &data.gravity.x, 0.01f);
  editor_ui::DragFloat3("Spawn Offset", &data.spawn_offset.x, 0.05f);
  editor_ui::DragFloat("Spawn Radius", &data.spawn_radius, 0.05f, 0.0f, 20.0f);
  editor_ui::DragFloat("Fade In", &data.fade_in_ratio, 0.01f, 0.0f, 1.0f);
  editor_ui::DragFloat("Fade Out", &data.fade_out_ratio, 0.01f, 0.0f, 1.0f);
  editor_ui::DragFloat("Emissive", &data.emissive_intensity, 0.1f, 0.0f, 10.0f);
  editor_ui::DragFloat("Soft Distance", &data.soft_distance, 0.01f, 0.01f, 5.0f);
  editor_ui::Checkbox("Loop", &data.loop);

  ApplyEditorData(data);
}
#endif
