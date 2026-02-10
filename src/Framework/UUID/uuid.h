#pragma once

// handle the marco for lib only
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <uuid_v4.h>

#pragma pop_macro("min")
#pragma pop_macro("max")

#include <string>
#include <string_view>

namespace framework {

inline UUIDv4::UUIDGenerator<std::mt19937_64>& GetUUIDGenerator() {
  static UUIDv4::UUIDGenerator<std::mt19937_64> generator;
  return generator;
}

class UUID {
 public:
  UUID() : uuid_(GetUUIDGenerator().getUUID()) {
  }

  static UUID FromString(std::string_view str) {
    return UUID(UUIDv4::UUID::fromStrFactory(std::string(str)));
  }

  std::string ToString() const {
    return uuid_.str();
  }

  bool operator==(const UUID& other) const {
    return uuid_ == other.uuid_;
  }

  const UUIDv4::UUID& GetRaw() const {
    return uuid_;
  }

 private:
  explicit UUID(const UUIDv4::UUID& raw) : uuid_(raw) {
  }
  UUIDv4::UUID uuid_;
};

}  // namespace framework

template <>
struct std::hash<framework::UUID> {
  size_t operator()(const framework::UUID& uuid) const noexcept {
    return uuid.GetRaw().hash();
  }
};
