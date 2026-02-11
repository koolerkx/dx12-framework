#pragma once

#include <filesystem>
#include <string>

#include "serialize_node.h"

namespace framework {

class SerializeDocument {
 public:
  SerializeDocument();
  ~SerializeDocument();

  SerializeNode& Root();
  std::string ToString() const;
  bool SaveToFile(const std::filesystem::path& path) const;

 private:
  SerializeNode root_;
};

}  // namespace framework
