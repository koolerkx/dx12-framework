#include "serialize_node_impl.h"

namespace framework {

SerializeNode::SerializeNode() : impl_(std::make_unique<Impl>()) {}
SerializeNode::~SerializeNode() = default;
SerializeNode::SerializeNode(SerializeNode&& other) noexcept = default;
SerializeNode& SerializeNode::operator=(SerializeNode&& other) noexcept = default;

SerializeNode::SerializeNode(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

SerializeNode& SerializeNode::Write(const std::string& key, const std::string& value) {
  impl_->node[key] = value;
  return *this;
}

SerializeNode& SerializeNode::Write(const std::string& key, const char* value) {
  impl_->node[key] = std::string(value);
  return *this;
}

SerializeNode& SerializeNode::Write(const std::string& key, float value) {
  impl_->node[key] = value;
  return *this;
}

SerializeNode& SerializeNode::Write(const std::string& key, int value) {
  impl_->node[key] = value;
  return *this;
}

SerializeNode& SerializeNode::Write(const std::string& key, uint32_t value) {
  impl_->node[key] = value;
  return *this;
}

SerializeNode& SerializeNode::Write(const std::string& key, bool value) {
  impl_->node[key] = value;
  return *this;
}

SerializeNode& SerializeNode::WriteVec2(const std::string& key, float x, float y) {
  YAML::Node seq(YAML::NodeType::Sequence);
  seq.SetStyle(YAML::EmitterStyle::Flow);
  seq.push_back(x);
  seq.push_back(y);
  impl_->node[key] = seq;
  return *this;
}

SerializeNode& SerializeNode::WriteVec3(const std::string& key, float x, float y, float z) {
  YAML::Node seq(YAML::NodeType::Sequence);
  seq.SetStyle(YAML::EmitterStyle::Flow);
  seq.push_back(x);
  seq.push_back(y);
  seq.push_back(z);
  impl_->node[key] = seq;
  return *this;
}

SerializeNode& SerializeNode::WriteVec4(const std::string& key, float x, float y, float z, float w) {
  YAML::Node seq(YAML::NodeType::Sequence);
  seq.SetStyle(YAML::EmitterStyle::Flow);
  seq.push_back(x);
  seq.push_back(y);
  seq.push_back(z);
  seq.push_back(w);
  impl_->node[key] = seq;
  return *this;
}

SerializeNode SerializeNode::BeginMap(const std::string& key) {
  YAML::Node child(YAML::NodeType::Map);
  impl_->node[key] = child;
  return SerializeNode(std::make_unique<Impl>(impl_->node[key]));
}

SerializeNode SerializeNode::BeginSequence(const std::string& key) {
  YAML::Node seq(YAML::NodeType::Sequence);
  impl_->node[key] = seq;
  return SerializeNode(std::make_unique<Impl>(impl_->node[key]));
}

SerializeNode SerializeNode::AddSequenceElement() {
  YAML::Node element(YAML::NodeType::Map);
  impl_->node.push_back(element);
  return SerializeNode(std::make_unique<Impl>(impl_->node[impl_->node.size() - 1]));
}

SerializeNode::Impl& SerializeNode::GetImpl() {
  return *impl_;
}

const SerializeNode::Impl& SerializeNode::GetImpl() const {
  return *impl_;
}

}  // namespace framework
