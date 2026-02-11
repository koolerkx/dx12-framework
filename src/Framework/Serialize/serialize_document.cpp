#include "serialize_document.h"

#include <fstream>

#include "serialize_node_impl.h"

namespace framework {

SerializeDocument::SerializeDocument() = default;
SerializeDocument::~SerializeDocument() = default;

SerializeNode& SerializeDocument::Root() {
  return root_;
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
