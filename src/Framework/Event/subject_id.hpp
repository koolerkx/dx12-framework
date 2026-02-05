/**
 * @file SubjectID.hpp
 * @brief Strong-typed entity ID wrapper for targeted event dispatch.
 * @details Prevents accidental mixing of raw uint64_t with entity IDs,
 *          provides type safety for EventBus::SubscribeTargeted/EmitTargeted.
 */

#pragma once

#include <cstdint>
#include <functional>

struct SubjectID {
  uint64_t value;

  explicit SubjectID(uint64_t v) : value(v) {
  }

  bool operator==(const SubjectID& other) const {
    return value == other.value;
  }

  bool operator!=(const SubjectID& other) const {
    return value != other.value;
  }
};

namespace std {
template <>
struct hash<SubjectID> {
  size_t operator()(const SubjectID& id) const noexcept {
    return std::hash<uint64_t>{}(id.value);
  }
};
}  // namespace std
