#pragma once
#include <DirectXMath.h>

#include <functional>

#include "Component/component.h"
#include "Component/sprite_sheet_helper.h"

class SpriteRenderer;

enum class PlayDirection : uint8_t { Forward, Backward };

class AnimatedSprite : public Component<AnimatedSprite> {
 public:
  using AnimationEndCallback = std::function<void()>;

  explicit AnimatedSprite(GameObject* owner);

  void OnStart() override;
  void OnUpdate(float dt) override;

  // Animation Configuration
  void SetSpriteSheetConfig(const SpriteSheet::FrameConfig& config);
  void SetFrameCount(uint32_t frame_count);
  void SetFramesPerSecond(float fps);

  // Playback Control
  void Play();
  void Stop();
  void Pause();
  void Resume();

  // Animation Settings
  void SetLoopEnabled(bool enabled);
  void SetPlayDirection(PlayDirection direction);
  void SetPlaySpeed(float speed_multiplier);
  void SetPlayOnStart(bool enabled);

  // State Queries
  bool IsPlaying() const;
  bool IsLooping() const;
  uint32_t GetCurrentFrame() const;
  float GetProgress() const;

  // Callback
  void SetAnimationEndCallback(AnimationEndCallback callback);

 private:
  void AdvanceFrame(float dt);
  void UpdateSpriteRendererUV();
  void ClampToValidFrameRange();
  void OnAnimationComplete();

 private:
  SpriteSheet::FrameConfig sheet_config_;
  uint32_t frame_count_ = 1;
  float seconds_per_frame_ = 0.1f;
  float accumulated_time_ = 0.0f;
  float play_speed_ = 1.0f;

  bool is_playing_ = false;
  bool loop_enabled_ = true;
  bool play_on_start_ = false;
  PlayDirection play_direction_ = PlayDirection::Forward;

  uint32_t current_frame_ = 0;
  AnimationEndCallback on_animation_end_;

  SpriteRenderer* sprite_renderer_ = nullptr;
};
