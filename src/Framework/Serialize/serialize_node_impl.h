#pragma once

#include <yaml-cpp/yaml.h>

#include "serialize_node.h"

namespace framework {

// pimpl pattern, the file outside no need to know the yaml-cpp
class SerializeNode::Impl {
 public:
  YAML::Node node;

  Impl() : node(YAML::NodeType::Map) {
  }
  explicit Impl(YAML::Node existing) : node(std::move(existing)) {
  }
};

}  // namespace framework
