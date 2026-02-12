#pragma once
#include <cstdint>
#include <functional>

#include "Component/sprite_sheet_helper.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"

using Math::Vector2;

enum class PlayDirection : uint8_t { Forward, Backward };

class SpriteSheetAnimator {
 public:
  using AnimationEndCallback = std::function<void()>;

  struct UVResult {
    Vector2 uv_offset = {0.0f, 0.0f};
    Vector2 uv_scale = {1.0f, 1.0f};
  };

  bool Update(float dt);
  UVResult GetCurrentUV() const;

  void SetSpriteSheetConfig(const SpriteSheet::FrameConfig& config);
  void SetFrameCount(uint32_t frame_count);
  void SetFramesPerSecond(float fps);

  void Play();
  void Stop();
  void Pause();
  void Resume();

  void SetLoopEnabled(bool enabled);
  void SetPlayDirection(PlayDirection direction);
  void SetPlaySpeed(float speed_multiplier);

  bool IsPlaying() const;
  bool IsLooping() const;
  uint32_t GetCurrentFrame() const;
  float GetProgress() const;

  void Serialize(framework::SerializeNode& node) const;
  void Deserialize(const framework::SerializeNode& node);

  void SetAnimationEndCallback(AnimationEndCallback callback);

 private:
  void AdvanceFrame(float dt);
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
  PlayDirection play_direction_ = PlayDirection::Forward;

  uint32_t current_frame_ = 0;
  AnimationEndCallback on_animation_end_;
};
