#pragma once

#include <memory>
#include <string>

namespace framework {

class SerializeNode {
 public:
  SerializeNode();
  ~SerializeNode();
  SerializeNode(SerializeNode&& other) noexcept;
  SerializeNode& operator=(SerializeNode&& other) noexcept;

  SerializeNode(const SerializeNode&) = delete;
  SerializeNode& operator=(const SerializeNode&) = delete;

  SerializeNode& Write(const std::string& key, const std::string& value);
  SerializeNode& Write(const std::string& key, const char* value);
  SerializeNode& Write(const std::string& key, float value);
  SerializeNode& Write(const std::string& key, int value);
  SerializeNode& Write(const std::string& key, uint32_t value);
  SerializeNode& Write(const std::string& key, bool value);

  SerializeNode& WriteVec2(const std::string& key, float x, float y);
  SerializeNode& WriteVec3(const std::string& key, float x, float y, float z);
  SerializeNode& WriteVec4(const std::string& key, float x, float y, float z, float w);

  SerializeNode BeginMap(const std::string& key);
  SerializeNode BeginSequence(const std::string& key);
  SerializeNode AddSequenceElement();

  // Read API
  std::string ReadString(const std::string& key, const std::string& default_value = "") const;
  float ReadFloat(const std::string& key, float default_value = 0.0f) const;
  int ReadInt(const std::string& key, int default_value = 0) const;
  uint32_t ReadUint(const std::string& key, uint32_t default_value = 0) const;
  bool ReadBool(const std::string& key, bool default_value = false) const;

  bool ReadVec2(const std::string& key, float& x, float& y) const;
  bool ReadVec3(const std::string& key, float& x, float& y, float& z) const;
  bool ReadVec4(const std::string& key, float& x, float& y, float& z, float& w) const;

  bool HasKey(const std::string& key) const;
  SerializeNode GetMap(const std::string& key) const;
  SerializeNode GetSequence(const std::string& key) const;
  SerializeNode GetSequenceElement(size_t index) const;
  size_t GetSequenceSize() const;
  size_t GetSequenceSize(const std::string& key) const;

  class Impl;
  Impl& GetImpl();
  const Impl& GetImpl() const;

 private:
  explicit SerializeNode(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;

  friend class SerializeDocument;
};

}  // namespace framework
