#pragma once

#include <cstdint>

namespace Framework {

template <typename Tag>
struct Handle {
  uint32_t index = UINT32_MAX;
  uint32_t generation = 0;

  bool IsValid() const {
    return index != UINT32_MAX;
  }

  bool operator==(const Handle&) const = default;

  static Handle Invalid() {
    return {};
  }
};

}  // namespace Framework
