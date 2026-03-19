#include "bindless_instance_grouper.h"

#include <algorithm>
#include <cstring>
#include <unordered_map>

#include "Frame/dynamic_upload_buffer.h"
#include "Resource/Buffer/gpu_instance_data.h"

namespace {

struct GroupKey {
  const Material* material;
  uint32_t mesh_index;
  uint32_t mesh_generation;
  uint32_t material_handle_index;
  RenderTagMask tags;

  bool operator==(const GroupKey&) const = default;
};

struct GroupKeyHash {
  size_t operator()(const GroupKey& k) const {
    size_t h = std::hash<const void*>{}(k.material);
    h ^= std::hash<uint32_t>{}(k.mesh_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.mesh_generation) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.material_handle_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.tags) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

struct PrepassGroupKey {
  uint32_t mesh_index;
  uint32_t mesh_generation;

  bool operator==(const PrepassGroupKey&) const = default;
};

struct PrepassGroupKeyHash {
  size_t operator()(const PrepassGroupKey& k) const {
    size_t h = std::hash<uint32_t>{}(k.mesh_index);
    h ^= std::hash<uint32_t>{}(k.mesh_generation) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

bool IsGroupableBindlessCommand(const DrawCommand& cmd) {
  if (!cmd.UsesBindlessMesh()) return false;
  if (cmd.IsInstanced()) return false;
  if (cmd.IsStructuredInstanced()) return false;
  if (cmd.has_custom_data) return false;
  return true;
}

GPUInstanceData BuildInstanceData(const DrawCommand& cmd) {
  GPUInstanceData data;
  data.world = cmd.world_matrix;
  data.color = cmd.color;
  data.uv_offset = cmd.uv_offset;
  data.uv_scale = cmd.uv_scale;
  data.overlay_color = Math::Vector4(0, 0, 0, 0);
  return data;
}

template <typename Key, typename Hash>
void GroupByKey(std::vector<DrawCommand>& commands, DynamicUploadBuffer* allocator, auto make_key) {
  std::unordered_map<Key, std::vector<size_t>, Hash> groups;

  for (size_t i = 0; i < commands.size(); ++i) {
    if (!IsGroupableBindlessCommand(commands[i])) continue;
    groups[make_key(commands[i])].push_back(i);
  }

  for (auto& [key, indices] : groups) {
    if (indices.size() < 2) continue;

    uint32_t count = static_cast<uint32_t>(indices.size());
    size_t data_size = count * sizeof(GPUInstanceData);
    auto alloc = allocator->Allocate(data_size);

    auto* gpu_data = static_cast<GPUInstanceData*>(alloc.cpu_ptr);
    float min_depth = commands[indices[0]].depth;

    for (uint32_t i = 0; i < count; ++i) {
      gpu_data[i] = BuildInstanceData(commands[indices[i]]);
      min_depth = (std::min)(min_depth, commands[indices[i]].depth);
    }

    DrawCommand& representative = commands[indices[0]];
    representative.instance_buffer_address = alloc.gpu_ptr;
    representative.instance_count = count;
    representative.depth = min_depth;

    for (size_t i = indices.size() - 1; i >= 1; --i) {
      commands[indices[i]] = DrawCommand{};
    }
  }

  std::erase_if(commands, [](const DrawCommand& cmd) { return !cmd.material && !cmd.mesh && !cmd.UsesBindlessMesh(); });
}

}  // namespace

void BindlessInstanceGrouper::Group(std::vector<DrawCommand>& commands, DynamicUploadBuffer* allocator) {
  if (!allocator) return;

  GroupByKey<GroupKey, GroupKeyHash>(commands, allocator, [](const DrawCommand& cmd) -> GroupKey {
    return {
      .material = cmd.material,
      .mesh_index = cmd.mesh_handle.index,
      .mesh_generation = cmd.mesh_handle.generation,
      .material_handle_index = cmd.material_handle.index,
      .tags = cmd.tags,
    };
  });
}

void BindlessInstanceGrouper::GroupForPrepass(std::vector<DrawCommand>& commands, DynamicUploadBuffer* allocator) {
  if (!allocator) return;

  GroupByKey<PrepassGroupKey, PrepassGroupKeyHash>(commands, allocator, [](const DrawCommand& cmd) -> PrepassGroupKey {
    return {
      .mesh_index = cmd.mesh_handle.index,
      .mesh_generation = cmd.mesh_handle.generation,
    };
  });
}
