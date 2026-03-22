/**
 * @file mesh_data.h
 * @brief CPU-side mesh data types for mesh generation and GPU upload.
 */
#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>

#include "Framework/Logging/logger.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/vertex_data.h"

template <typename T>
concept MeshVertex = std::same_as<T, VertexData::ModelVertex> || std::same_as<T, VertexData::SpriteVertex>;

struct MeshData {
  using VertexStorage = std::variant<std::vector<VertexData::ModelVertex>, std::vector<VertexData::SpriteVertex>>;

  VertexStorage vertices;
  std::vector<uint32_t> indices;

  bool IsModel() const {
    return std::holds_alternative<std::vector<VertexData::ModelVertex>>(vertices);
  }

  bool IsSprite() const {
    return std::holds_alternative<std::vector<VertexData::SpriteVertex>>(vertices);
  }

  uint32_t GetVertexCount() const {
    return std::visit([](const auto& v) { return static_cast<uint32_t>(v.size()); }, vertices);
  }

  template <MeshVertex T>
  std::span<const T> GetVertices() const {
    if (auto* v = std::get_if<std::vector<T>>(&vertices)) return *v;
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Core, Logger::Here(), "[MeshData] GetVertices<{}> called on wrong vertex type", typeid(T).name());
    return {};
  }

  template <MeshVertex T>
  std::vector<T>& GetVertices() {
    if (!std::holds_alternative<std::vector<T>>(vertices)) {
      Logger::LogFormat(LogLevel::Error,
        LogCategory::Core,
        Logger::Here(),
        "[MeshData] GetVertices<{}> mutable called on wrong vertex type",
        typeid(T).name());
    }
    return std::get<std::vector<T>>(vertices);
  }
};

struct MeshAllocation {
  MeshHandle handle;
  bool success = false;
};
