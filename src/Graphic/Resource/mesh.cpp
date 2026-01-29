#include "mesh.h"

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList, uint32_t submeshIndex) const {
  auto vbv = vertexBuffer_.GetView();
  auto ibv = indexBuffer_.GetView();

  cmdList->IASetVertexBuffers(0, 1, &vbv);
  cmdList->IASetIndexBuffer(&ibv);

  const auto& submesh = subMeshes_[submeshIndex];
  cmdList->DrawIndexedInstanced(submesh.indexCount,
    1,  // instance count
    submesh.startIndexLocation,
    submesh.baseVertexLocation,
    0  // start instance
  );
}
