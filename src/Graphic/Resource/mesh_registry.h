#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "mesh.h"

class MeshRegistry {
 public:
  const Mesh* Register(const std::string& key, std::unique_ptr<Mesh> mesh) {
    auto [it, inserted] = meshes_.try_emplace(key, std::move(mesh));
    return it->second.get();
  }

  const Mesh* Find(const std::string& key) const {
    auto it = meshes_.find(key);
    return it != meshes_.end() ? it->second.get() : nullptr;
  }

  bool Contains(const std::string& key) const {
    return meshes_.contains(key);
  }

  void Clear() {
    meshes_.clear();
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;
};
