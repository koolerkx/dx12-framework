#pragma once
#include <cstdint>

enum class PlayState : uint8_t {
  Stopped,
  Playing,
  Paused,
};
