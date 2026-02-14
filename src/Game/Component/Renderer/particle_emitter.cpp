#include "particle_emitter.h"

#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"

thread_local std::mt19937 ParticleEmitter::rng_{std::random_device{}()};

ParticleEmitter::ParticleEmitter(GameObject* owner) : Component(owner), spawn_fn_(SpawnFromCenter()) {
}

ParticleEmitter::ParticleEmitter(GameObject* owner, const Props& props) : Component(owner) {
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
  spawn_fn_ = props.spawn_fn ? props.spawn_fn : SpawnFromCenter();

  render_settings_ = Rendering::RenderSettings::Transparent();
  render_settings_.blend_mode = props.blend_mode;
}

void ParticleEmitter::Play() {
  is_playing_ = true;
  emit_accumulator_ = 0.0f;
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
}

void ParticleEmitter::OnRender(FramePacket& packet) {
  if (!texture_ || particles_.empty()) return;

  auto* context = GetOwner()->GetContext();
  auto& material_mgr = context->GetGraphic()->GetMaterialManager();

  DrawCommand cmd;
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Rect);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteInstancedShader::ID, render_settings_);
  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

  cmd.instances.reserve(particles_.size());
  for (const auto& p : particles_) {
    Matrix4 billboard_rot = Matrix4::FaceTo(p.position, packet.main_camera.position, Vector3::Up);
    Matrix4 scale_mat = Matrix4::CreateScale({particle_size_.x * p.size, particle_size_.y * p.size, 1.0f});
    Matrix4 world = scale_mat * billboard_rot * Matrix4::CreateTranslation(p.position);

    cmd.instances.push_back({
      .world_matrix = world,
      .color = p.color,
      .uv_offset = {0, 0},
      .uv_scale = {1, 1},
    });
  }

  cmd.depth = Vector3::DistanceSquared(GetOwner()->GetTransform()->GetWorldPosition(), packet.main_camera.position);
  cmd.layer = RenderLayer::Transparent;
  cmd.depth_test = render_settings_.depth_test;
  cmd.depth_write = render_settings_.depth_write;
  packet.AddCommand(std::move(cmd));
}

void ParticleEmitter::EmitParticles(float dt) {
  emit_accumulator_ += dt;
  float interval = 1.0f / emit_rate_;

  while (emit_accumulator_ >= interval && particles_.size() < max_particles_) {
    emit_accumulator_ -= interval;

    std::uniform_real_distribution<float> speed_dist(start_speed_ - speed_variation_, start_speed_ + speed_variation_);

    Vector3 emitter_pos = GetOwner()->GetTransform()->GetWorldPosition();
    auto [offset, direction] = spawn_fn_(rng_);

    Particle p;
    p.position = emitter_pos + offset;
    p.velocity = direction * speed_dist(rng_);
    p.color = start_color_;
    p.size = 1.0f;
    p.lifetime = particle_lifetime_;
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
    p.position = p.position + p.velocity * dt;
    p.age += dt;
    p.color = Vector4::LerpClamped(start_color_, end_color_, p.GetLifetimeRatio());
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
