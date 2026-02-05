#include "sprite_sheet_animator.h"

#include <algorithm>
#include <cmath>

bool SpriteSheetAnimator::Update(float dt) {
  if (!is_playing_) {
    return false;
  }

  uint32_t previous_frame = current_frame_;
  AdvanceFrame(dt);
  return current_frame_ != previous_frame;
}

SpriteSheetAnimator::UVResult SpriteSheetAnimator::GetCurrentUV() const {
  if (sheet_config_.sheet_size.x == 0 || sheet_config_.sheet_size.y == 0) {
    return {{0.0f, 0.0f}, {1.0f, 1.0f}};
  }

  auto uv = SpriteSheet::CalculateFrameUV(sheet_config_, current_frame_);
  return {uv.uv_offset, uv.uv_scale};
}

void SpriteSheetAnimator::AdvanceFrame(float dt) {
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
        OnAnimationComplete();
        break;
      }
    }
  }
}

void SpriteSheetAnimator::Play() {
  current_frame_ = 0;
  accumulated_time_ = 0.0f;
  is_playing_ = true;
}

void SpriteSheetAnimator::Stop() {
  is_playing_ = false;
  current_frame_ = 0;
  accumulated_time_ = 0.0f;
}

void SpriteSheetAnimator::Pause() {
  is_playing_ = false;
}

void SpriteSheetAnimator::Resume() {
  if (frame_count_ > 0) {
    is_playing_ = true;
  }
}

void SpriteSheetAnimator::SetFramesPerSecond(float fps) {
  if (fps <= 0.0f || !std::isfinite(fps)) {
    seconds_per_frame_ = 0.1f;
    return;
  }
  seconds_per_frame_ = 1.0f / fps;
}

void SpriteSheetAnimator::SetSpriteSheetConfig(const SpriteSheet::FrameConfig& config) {
  sheet_config_ = config;
}

void SpriteSheetAnimator::SetFrameCount(uint32_t count) {
  frame_count_ = (std::max)(1u, count);
  ClampToValidFrameRange();
}

void SpriteSheetAnimator::ClampToValidFrameRange() {
  if (current_frame_ >= frame_count_) {
    current_frame_ = frame_count_ - 1;
  }
}

void SpriteSheetAnimator::OnAnimationComplete() {
  is_playing_ = false;

  auto callback = std::move(on_animation_end_);
  on_animation_end_ = nullptr;

  if (callback) {
    callback();
  }
}

void SpriteSheetAnimator::SetAnimationEndCallback(AnimationEndCallback callback) {
  on_animation_end_ = std::move(callback);
}

bool SpriteSheetAnimator::IsPlaying() const {
  return is_playing_;
}

bool SpriteSheetAnimator::IsLooping() const {
  return loop_enabled_;
}

uint32_t SpriteSheetAnimator::GetCurrentFrame() const {
  return current_frame_;
}

float SpriteSheetAnimator::GetProgress() const {
  if (frame_count_ <= 1) return 1.0f;
  return static_cast<float>(current_frame_) / static_cast<float>(frame_count_ - 1);
}

void SpriteSheetAnimator::SetLoopEnabled(bool enabled) {
  loop_enabled_ = enabled;
}

void SpriteSheetAnimator::SetPlayDirection(PlayDirection direction) {
  play_direction_ = direction;
}

void SpriteSheetAnimator::SetPlaySpeed(float speed) {
  play_speed_ = (std::max)(0.0f, speed);
}
