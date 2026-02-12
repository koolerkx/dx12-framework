#include "serialize_document.h"

#include <fstream>

#include "serialize_node_impl.h"

namespace framework {

SerializeDocument::SerializeDocument() = default;
SerializeDocument::~SerializeDocument() = default;

SerializeNode& SerializeDocument::Root() {
  return root_;
}

const SerializeNode& SerializeDocument::Root() const {
  return root_;
}

bool SerializeDocument::LoadFromFile(const std::filesystem::path& path) {
  if (!std::filesystem::exists(path)) return false;
  try {
    YAML::Node loaded = YAML::LoadFile(path.string());
    root_ = SerializeNode(std::make_unique<SerializeNode::Impl>(loaded));
    return true;
  } catch (const YAML::Exception&) {
    return false;
  }
}

std::string SerializeDocument::ToString() const {
  YAML::Emitter emitter;
  emitter << root_.GetImpl().node;
  return emitter.c_str();
}

bool SerializeDocument::SaveToFile(const std::filesystem::path& path) const {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream file(path, std::ios::out | std::ios::trunc);
  if (!file.is_open()) return false;
  file << ToString();
  return file.good();
}

}  // namespace framework
