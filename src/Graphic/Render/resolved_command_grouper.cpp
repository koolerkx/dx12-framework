#include "resolved_command_grouper.h"

#include <algorithm>
#include <unordered_map>

namespace {

struct GroupKey {
  const Material* material;
  D3D12_GPU_VIRTUAL_ADDRESS vbv_location;
  D3D12_GPU_VIRTUAL_ADDRESS ibv_location;
  uint32_t index_count;
  uint32_t index_offset;
  int32_t vertex_offset;
  uint32_t material_handle_index;
  RenderTagMask tags;

  bool operator==(const GroupKey&) const = default;
};

struct GroupKeyHash {
  size_t operator()(const GroupKey& k) const {
    size_t h = std::hash<const void*>{}(k.material);
    h ^= std::hash<uint64_t>{}(k.vbv_location) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint64_t>{}(k.ibv_location) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.index_count) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.index_offset) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<int32_t>{}(k.vertex_offset) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.material_handle_index) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.tags) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

struct PrepassGroupKey {
  D3D12_GPU_VIRTUAL_ADDRESS vbv_location;
  D3D12_GPU_VIRTUAL_ADDRESS ibv_location;
  uint32_t index_count;
  uint32_t index_offset;
  int32_t vertex_offset;

  bool operator==(const PrepassGroupKey&) const = default;
};

struct PrepassGroupKeyHash {
  size_t operator()(const PrepassGroupKey& k) const {
    size_t h = std::hash<uint64_t>{}(k.vbv_location);
    h ^= std::hash<uint64_t>{}(k.ibv_location) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.index_count) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(k.index_offset) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<int32_t>{}(k.vertex_offset) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

bool IsGroupable(const ResolvedDrawCommand& cmd) {
  if (cmd.custom_data.active) return false;
  if (cmd.instance_count > 1) return false;
  return true;
}

template <typename Key, typename Hash>
void GroupByKey(std::vector<ResolvedDrawCommand>& commands, auto make_key) {
  std::unordered_map<Key, std::vector<size_t>, Hash> groups;

  for (size_t i = 0; i < commands.size(); ++i) {
    if (!IsGroupable(commands[i])) continue;
    groups[make_key(commands[i])].push_back(i);
  }

  for (auto& [key, indices] : groups) {
    if (indices.size() < 2) continue;

    uint32_t count = static_cast<uint32_t>(indices.size());
    float min_depth = commands[indices[0]].depth;

    std::vector<uint32_t> object_indices;
    object_indices.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
      object_indices.push_back(commands[indices[i]].object_index);
      min_depth = (std::min)(min_depth, commands[indices[i]].depth);
    }

    ResolvedDrawCommand& representative = commands[indices[0]];
    representative.grouped_object_indices = std::move(object_indices);
    representative.instance_count = count;
    representative.depth = min_depth;

    for (size_t i = indices.size() - 1; i >= 1; --i) {
      commands[indices[i]] = ResolvedDrawCommand{};
    }
  }

  std::erase_if(commands, [](const ResolvedDrawCommand& cmd) { return !cmd.material && cmd.geometry.index_count == 0; });
}

}  // namespace

void ResolvedCommandGrouper::Group(std::vector<ResolvedDrawCommand>& commands) {
  GroupByKey<GroupKey, GroupKeyHash>(commands, [](const ResolvedDrawCommand& cmd) -> GroupKey {
    return {
      .material = cmd.material,
      .vbv_location = cmd.geometry.vbv.BufferLocation,
      .ibv_location = cmd.geometry.ibv.BufferLocation,
      .index_count = cmd.geometry.index_count,
      .index_offset = cmd.geometry.index_offset,
      .vertex_offset = cmd.geometry.vertex_offset,
      .material_handle_index = cmd.material_handle.index,
      .tags = cmd.tags,
    };
  });
}

void ResolvedCommandGrouper::GroupForPrepass(std::vector<ResolvedDrawCommand>& commands) {
  GroupByKey<PrepassGroupKey, PrepassGroupKeyHash>(commands, [](const ResolvedDrawCommand& cmd) -> PrepassGroupKey {
    return {
      .vbv_location = cmd.geometry.vbv.BufferLocation,
      .ibv_location = cmd.geometry.ibv.BufferLocation,
      .index_count = cmd.geometry.index_count,
      .index_offset = cmd.geometry.index_offset,
      .vertex_offset = cmd.geometry.vertex_offset,
    };
  });
}
