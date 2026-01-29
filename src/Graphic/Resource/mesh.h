/**
 * @file mesh.h
 * @author Kooler Fan
 * @brief Mesh class
 * The mesh is a collection of vertices and indices.
 * The responsibility of the mesh class here is to provide resource management and draw encapsulation.
 * The submesh means a segment of index. Designed for multi-material support.
 * ---
 * メッシュは頂点とインデックスを表します。
 * ここでのメッシュクラスの責務は、リソース管理および描画処理のカプセル化を提供することです。
 * サブメッシュはインデックスの一部区間を指し、マルチマテリアル対応のために設計されています。
 * @code {.cpp}
 * Mesh quadMesh;
 * Mesh cubeMesh;
 * Mesh sphereMesh;
 *
 * quadMesh.Create(device, quadVerts, 4, quadIndices, 6);
 * cubeMesh.Create(device, cubeVerts, 8, cubeIndices, 36);
 * sphereMesh.Create(device, sphereVerts, 100, sphereIndices, 300);

 * quadMesh.Draw(cmdList);
 * cubeMesh.Draw(cmdList);
 * sphereMesh.Draw(cmdList);
 * @endcode
 *
 * @note Now only support single mesh, refactor to support multiple submesh in future
 */

#pragma once

#include <d3d12.h>

#include <cstdint>
#include <vector>

#include "Command/buffer.h"

class Mesh {
 public:
  struct SubMesh {
    uint32_t indexCount;
    uint32_t startIndexLocation;
    int32_t baseVertexLocation;
  };

  template <typename VertexType>
  bool Create(ID3D12Device* device, const VertexType* vertices, size_t vertexCount, const uint16_t* indices, size_t indexCount) {
    if (!vertexBuffer_.Create(device, vertices, vertexCount)) {
      return false;
    }

    if (!indexBuffer_.Create(device, indices, indexCount)) {
      return false;
    }

    // default mesh is a single submesh
    SubMesh submesh;
    submesh.indexCount = static_cast<uint32_t>(indexCount);
    submesh.startIndexLocation = 0;
    submesh.baseVertexLocation = 0;
    subMeshes_.push_back(submesh);

    return true;
  }

  void Draw(ID3D12GraphicsCommandList* cmdList, uint32_t submeshIndex = 0) const;

  // Getters for advanced usage
  const VertexBuffer& GetVertexBuffer() const {
    return vertexBuffer_;
  }
  const IndexBuffer& GetIndexBuffer() const {
    return indexBuffer_;
  }
  size_t GetSubMeshCount() const {
    return subMeshes_.size();
  }
  const SubMesh& GetSubMesh(size_t index = 0) const {
    return subMeshes_[index];
  }

 private:
  VertexBuffer vertexBuffer_;
  IndexBuffer indexBuffer_;
  std::vector<SubMesh> subMeshes_;
};
