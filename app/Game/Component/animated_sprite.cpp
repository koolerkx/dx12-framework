#include "animated_sprite.h"

#include <algorithm>
#include <cmath>

#include "Component/sprite_renderer.h"
#include "game_object.h"

AnimatedSprite::AnimatedSprite(GameObject* owner) : Component(owner) {
}

void AnimatedSprite::OnStart() {
  sprite_renderer_ = GetOwner()->GetComponent<SpriteRenderer>();

  if (!sprite_renderer_) {
    return;
  }

  UpdateSpriteRendererUV();

  if (play_on_start_) {
    Play();
  }
}

void AnimatedSprite::OnUpdate(float dt) {
  if (!is_playing_ || !sprite_renderer_) {
    return;
  }

  AdvanceFrame(dt);
  UpdateSpriteRendererUV();
}

void AnimatedSprite::AdvanceFrame(float dt) {
  accumulated_time_ += dt * play_speed_;

  while (accumulated_time_ >= seconds_per_frame_) {
    accumulated_time_ -= seconds_per_frame_;

    if (play_direction_ == PlayDirection::Forward) {
      current_frame_++;
    } else {
      current_frame_ = (current_frame_ == 0) ? frame_count_ - 1 : current_frame_ - 1;
    }

    bool reached_end = (play_direction_ == PlayDirection::Forward) ? (current_frame_ >= frame_count_)
                                                                   : (current_frame_ <= 0 && accumulated_time_ < seconds_per_frame_);

    if (reached_end) {
      if (loop_enabled_) {
        current_frame_ = (play_direction_ == PlayDirection::Forward) ? 0 : frame_count_ - 1;
      } else {
        current_frame_ = (play_direction_ == PlayDirection::Forward) ? frame_count_ - 1 : 0;
        is_playing_ = false;
        OnAnimationComplete();
        break;
      }
    }
  }
}

void AnimatedSprite::UpdateSpriteRendererUV() {
  if (!sprite_renderer_) {
    sprite_renderer_ = GetOwner()->GetComponent<SpriteRenderer>();
    if (!sprite_renderer_) return;
  }

  // Check if sheet config is valid (not zero-sized)
  if (sheet_config_.sheet_size.x == 0 || sheet_config_.sheet_size.y == 0) {
    // Config not set yet, use default full texture
    sprite_renderer_->SetUVOffset({0.0f, 0.0f});
    sprite_renderer_->SetUVScale({1.0f, 1.0f});
    return;
  }

  auto uv = SpriteSheet::CalculateFrameUV(sheet_config_, current_frame_);
  sprite_renderer_->SetUVOffset(uv.uv_offset);
  sprite_renderer_->SetUVScale(uv.uv_scale);
}

void AnimatedSprite::Play() {
  current_frame_ = 0;
  accumulated_time_ = 0.0f;
  is_playing_ = true;
  UpdateSpriteRendererUV();
}

void AnimatedSprite::Stop() {
  is_playing_ = false;
  current_frame_ = 0;
  accumulated_time_ = 0.0f;
  UpdateSpriteRendererUV();
}

void AnimatedSprite::Pause() {
  is_playing_ = false;
}

void AnimatedSprite::Resume() {
  if (frame_count_ > 0) {
    is_playing_ = true;
  }
}

void AnimatedSprite::SetFramesPerSecond(float fps) {
  if (fps <= 0.0f || !std::isfinite(fps)) {
    seconds_per_frame_ = 0.1f;
    return;
  }
  seconds_per_frame_ = 1.0f / fps;
}

void AnimatedSprite::SetSpriteSheetConfig(const SpriteSheet::FrameConfig& config) {
  sheet_config_ = config;
  // Update UV immediately if renderer is available
  UpdateSpriteRendererUV();
}

void AnimatedSprite::SetFrameCount(uint32_t count) {
  frame_count_ = (std::max)(1u, count);
  ClampToValidFrameRange();
}

void AnimatedSprite::ClampToValidFrameRange() {
  if (current_frame_ >= frame_count_) {
    current_frame_ = frame_count_ - 1;
  }
}

void AnimatedSprite::OnAnimationComplete() {
  is_playing_ = false;

  auto callback = std::move(on_animation_end_);
  on_animation_end_ = nullptr;

  if (callback) {
    callback();
  }
}

void AnimatedSprite::SetAnimationEndCallback(AnimationEndCallback callback) {
  on_animation_end_ = std::move(callback);
}

bool AnimatedSprite::IsPlaying() const {
  return is_playing_;
}

bool AnimatedSprite::IsLooping() const {
  return loop_enabled_;
}

uint32_t AnimatedSprite::GetCurrentFrame() const {
  return current_frame_;
}

float AnimatedSprite::GetProgress() const {
  if (frame_count_ <= 1) return 1.0f;
  return static_cast<float>(current_frame_) / static_cast<float>(frame_count_ - 1);
}

void AnimatedSprite::SetLoopEnabled(bool enabled) {
  loop_enabled_ = enabled;
}

void AnimatedSprite::SetPlayDirection(PlayDirection direction) {
  play_direction_ = direction;
}

void AnimatedSprite::SetPlaySpeed(float speed) {
  play_speed_ = (std::max)(0.0f, speed);
}

void AnimatedSprite::SetPlayOnStart(bool enabled) {
  play_on_start_ = enabled;
}
