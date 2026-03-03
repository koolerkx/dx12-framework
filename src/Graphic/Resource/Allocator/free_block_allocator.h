#pragma once

#include <cstdint>
#include <set>

class FreeBlockAllocator {
 public:
  static constexpr uint32_t INVALID_OFFSET = UINT32_MAX;

  FreeBlockAllocator() = default;

  void Initialize(uint32_t total_size);

  uint32_t Allocate(uint32_t count);
  void Free(uint32_t offset, uint32_t count);

  uint32_t GetTotalSize() const {
    return total_size_;
  }
  uint32_t GetUsedSize() const {
    return total_size_ - free_size_;
  }
  uint32_t GetFreeSize() const {
    return free_size_;
  }
  uint32_t GetLargestFreeBlock() const;

 private:
  struct FreeBlock {
    uint32_t offset;
    uint32_t size;

    bool operator<(const FreeBlock& other) const {
      return offset < other.offset;
    }
  };

  std::set<FreeBlock> free_blocks_;
  uint32_t total_size_ = 0;
  uint32_t free_size_ = 0;
};
