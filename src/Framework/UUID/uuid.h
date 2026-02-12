#pragma once

// handle the macro for lib only
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#pragma warning(push)
#pragma warning(disable : 4244)  // uuid_v4.h: int -> uint16_t

#include <uuid_v4.h>

#pragma warning(pop)
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
  // BUG workaround: UUIDv4::UUID::hash() reads out-of-bounds (data+64 instead of data+8)
  // UUIDv4::UUID layout: alignas(128) uint8_t data[16] (only member, standard layout)
  size_t operator()(const framework::UUID& uuid) const noexcept {
    const auto* p = reinterpret_cast<const uint64_t*>(&uuid.GetRaw());
    return p[0] ^ p[1];
  }
};
