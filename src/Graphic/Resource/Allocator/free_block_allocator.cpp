#include "Resource/Allocator/free_block_allocator.h"

void FreeBlockAllocator::Initialize(uint32_t total_size) {
  free_blocks_.clear();
  total_size_ = total_size;
  free_size_ = total_size;

  if (total_size > 0) {
    free_blocks_.insert({0, total_size});
  }
}

uint32_t FreeBlockAllocator::Allocate(uint32_t count) {
  if (count == 0 || count > free_size_) return INVALID_OFFSET;

  // Best-fit search: find smallest block that fits
  auto best = free_blocks_.end();
  for (auto it = free_blocks_.begin(); it != free_blocks_.end(); ++it) {
    if (it->size >= count) {
      if (best == free_blocks_.end() || it->size < best->size) {
        best = it;
        if (it->size == count) break;  // Exact fit
      }
    }
  }

  if (best == free_blocks_.end()) return INVALID_OFFSET;

  uint32_t offset = best->offset;
  uint32_t remaining = best->size - count;

  free_blocks_.erase(best);

  if (remaining > 0) {
    free_blocks_.insert({offset + count, remaining});
  }

  free_size_ -= count;
  return offset;
}

void FreeBlockAllocator::Free(uint32_t offset, uint32_t count) {
  if (count == 0) return;

  uint32_t new_offset = offset;
  uint32_t new_size = count;

  // Merge with right neighbor
  auto right = free_blocks_.upper_bound({offset, 0});
  if (right != free_blocks_.end() && right->offset == new_offset + new_size) {
    new_size += right->size;
    right = free_blocks_.erase(right);
  }

  // Merge with left neighbor
  if (right != free_blocks_.begin()) {
    auto left = std::prev(right);
    if (left->offset + left->size == new_offset) {
      new_offset = left->offset;
      new_size += left->size;
      free_blocks_.erase(left);
    }
  }

  free_blocks_.insert({new_offset, new_size});
  free_size_ += count;
}

uint32_t FreeBlockAllocator::GetLargestFreeBlock() const {
  uint32_t largest = 0;
  for (const auto& block : free_blocks_) {
    if (block.size > largest) largest = block.size;
  }
  return largest;
}
