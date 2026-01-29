#pragma once
#include <memory>
#include <string>

template <typename T>
class AssetHandle {
 public:
  AssetHandle() = default;
  explicit AssetHandle(std::shared_ptr<T> resource, std::string path) : resource_(resource), path_(std::move(path)) {
  }

  T* Get() const {
    return resource_.get();
  }
  T* operator->() const {
    return resource_.get();
  }
  bool IsValid() const {
    return resource_ != nullptr;
  }
  const std::string& GetPath() const {
    return path_;
  }

 private:
  std::shared_ptr<T> resource_;
  std::string path_;
};
