#include "timer_updater.h"

#include <cassert>

TimerUpdater::TimerUpdater(float fixedHz, float maxClamp, int maxSteps) {
  // Parameter validation (assert + runtime fallback)
  assert(fixedHz > 0.0f && "fixedHz must be > 0");
  if (fixedHz <= 0.0f) fixedHz = 60.0f;

  assert(maxClamp >= 0.0f && "maxClamp must be >= 0");
  if (maxClamp < 0.0f) maxClamp = 0.25f;

  assert(maxSteps >= 1 && "maxSteps must be >= 1");
  if (maxSteps < 1) maxSteps = 1;

  fixed_dt_ = secondsf(1.0f / fixedHz);
  max_frame_clamp_ = maxClamp;
  max_sub_steps_ = maxSteps;

  reset();
  tick_thread_id_ = std::this_thread::get_id();
}

void TimerUpdater::reset() {
  prev_ = clock::now();
  accumulator_ = secondsf::zero();
  alpha_ = 0.0f;
  time_scale_ = 1.0f;
  paused_ = false;
  total_unscaled_ = secondsf::zero();
  total_scaled_ = secondsf::zero();
}

void TimerUpdater::SetFixedHz(float hz) {
  assert(hz > 0.0f && "fixedHz must be > 0");
  if (hz <= 0.0f) hz = 60.0f;
  fixed_dt_ = secondsf(1.0f / hz);
}

void TimerUpdater::SetTimeScale(float s) {
  if (s < 0.0f) s = 0.0f;
  if (s > 10.0f) s = 10.0f;
  time_scale_ = s;
}

void TimerUpdater::SetMaxFrameClamp(float seconds) {
  assert(seconds >= 0.0f);
  if (seconds < 0.0f) seconds = 0.0f;
  max_frame_clamp_ = seconds;
}

void TimerUpdater::SetMaxSubSteps(int n) {
  assert(n >= 1);
  if (n < 1) n = 1;
  max_sub_steps_ = n;
}

void TimerUpdater::SetPaused(bool p) {
  paused_ = p;
}

void TimerUpdater::BindTickThreadToCurrent() {
  tick_thread_id_ = std::this_thread::get_id();
}
