#include "object_data_buffer.h"

#include <cstring>

#include "Frame/dynamic_upload_buffer.h"

uint32_t ObjectDataBuffer::Append(const ObjectData& data) {
  uint32_t index = static_cast<uint32_t>(staging_.size());
  staging_.push_back(data);
  return index;
}

void ObjectDataBuffer::Upload(DynamicUploadBuffer& allocator) {
  if (staging_.empty()) {
    gpu_address_ = 0;
    return;
  }

  size_t byte_size = staging_.size() * sizeof(ObjectData);
  auto alloc = allocator.Allocate(byte_size);
  std::memcpy(alloc.cpu_ptr, staging_.data(), byte_size);
  gpu_address_ = alloc.gpu_ptr;
}

void ObjectDataBuffer::Reset() {
  staging_.clear();
  gpu_address_ = 0;
}
