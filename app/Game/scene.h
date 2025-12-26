#pragma once
#include <DirectXMath.h>

#include <memory>

#include "render_world.h"

class Graphic;
struct Texture;

class IScene {
 public:
  virtual ~IScene() = default;

  // Load textures/assets
  virtual void OnEnter(Graphic& graphic) = 0;

  // Called every frame
  virtual void OnUpdate(float dt) = 0;

  // Called every tick
  virtual void OnFixedUpdate(float dt) = 0;

  // Push the data to render
  virtual void OnRender(RenderWorld& world) = 0;

  // Release unique resources
  virtual void OnExit() = 0;
};

class TestScene : public IScene {
 public:
  TestScene() = default;
  virtual ~TestScene() = default;

  void OnEnter(Graphic& graphic) override;
  void OnUpdate(float dt) override;
  void OnFixedUpdate(float) override {};
  void OnRender(RenderWorld& world) override;
  void OnExit() override;

 private:
  // Scene-specific resources
  std::shared_ptr<Texture> texture_background_;
  std::shared_ptr<Texture> texture_character_;

  // Simple game state
  DirectX::XMFLOAT2 character_pos_ = {0.0f, 0.0f};
};
