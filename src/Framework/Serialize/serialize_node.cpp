#include "serialize_node_impl.h"

namespace framework {

SerializeNode::SerializeNode() : impl_(std::make_unique<Impl>()) {
}
SerializeNode::~SerializeNode() = default;
SerializeNode::SerializeNode(SerializeNode&& other) noexcept = default;
SerializeNode& SerializeNode::operator=(SerializeNode&& other) noexcept = default;

SerializeNode::SerializeNode(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {
}

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

std::string SerializeNode::ReadString(const std::string& key, const std::string& default_value) const {
  if (!impl_->node[key]) return default_value;
  return impl_->node[key].as<std::string>(default_value);
}

float SerializeNode::ReadFloat(const std::string& key, float default_value) const {
  if (!impl_->node[key]) return default_value;
  return impl_->node[key].as<float>(default_value);
}

int SerializeNode::ReadInt(const std::string& key, int default_value) const {
  if (!impl_->node[key]) return default_value;
  return impl_->node[key].as<int>(default_value);
}

uint32_t SerializeNode::ReadUint(const std::string& key, uint32_t default_value) const {
  if (!impl_->node[key]) return default_value;
  return impl_->node[key].as<uint32_t>(default_value);
}

bool SerializeNode::ReadBool(const std::string& key, bool default_value) const {
  if (!impl_->node[key]) return default_value;
  return impl_->node[key].as<bool>(default_value);
}

bool SerializeNode::ReadVec2(const std::string& key, float& x, float& y) const {
  auto seq = impl_->node[key];
  if (!seq || !seq.IsSequence() || seq.size() < 2) return false;
  x = seq[0].as<float>();
  y = seq[1].as<float>();
  return true;
}

bool SerializeNode::ReadVec3(const std::string& key, float& x, float& y, float& z) const {
  auto seq = impl_->node[key];
  if (!seq || !seq.IsSequence() || seq.size() < 3) return false;
  x = seq[0].as<float>();
  y = seq[1].as<float>();
  z = seq[2].as<float>();
  return true;
}

bool SerializeNode::ReadVec4(const std::string& key, float& x, float& y, float& z, float& w) const {
  auto seq = impl_->node[key];
  if (!seq || !seq.IsSequence() || seq.size() < 4) return false;
  x = seq[0].as<float>();
  y = seq[1].as<float>();
  z = seq[2].as<float>();
  w = seq[3].as<float>();
  return true;
}

bool SerializeNode::HasKey(const std::string& key) const {
  return impl_->node[key] && impl_->node[key].IsDefined();
}

SerializeNode SerializeNode::GetMap(const std::string& key) const {
  if (impl_->node[key] && impl_->node[key].IsMap()) {
    return SerializeNode(std::make_unique<Impl>(impl_->node[key]));
  }
  return SerializeNode();
}

SerializeNode SerializeNode::GetSequence(const std::string& key) const {
  if (impl_->node[key] && impl_->node[key].IsSequence()) {
    return SerializeNode(std::make_unique<Impl>(impl_->node[key]));
  }
  return SerializeNode();
}

size_t SerializeNode::GetSequenceSize() const {
  if (!impl_->node.IsSequence()) return 0;
  return impl_->node.size();
}

SerializeNode SerializeNode::GetSequenceElement(size_t index) const {
  if (impl_->node.IsSequence() && index < impl_->node.size()) {
    return SerializeNode(std::make_unique<Impl>(impl_->node[index]));
  }
  return SerializeNode();
}

size_t SerializeNode::GetSequenceSize(const std::string& key) const {
  auto seq = impl_->node[key];
  if (!seq || !seq.IsSequence()) return 0;
  return seq.size();
}

SerializeNode::Impl& SerializeNode::GetImpl() {
  return *impl_;
}

const SerializeNode::Impl& SerializeNode::GetImpl() const {
  return *impl_;
}

}  // namespace framework
