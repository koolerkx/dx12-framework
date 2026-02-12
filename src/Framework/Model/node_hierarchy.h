#pragma once

#include <DirectXMath.h>

#include <string>
#include <vector>

#include "model_types.h"

namespace Model {

/**
 * @brief Scene graph node with hierarchical transformation
 *
 * Represents a node in the scene hierarchy from the model file.
 * Each node can have multiple meshes and child nodes.
 */
struct Node {
  std::string name;
  NodeTransform transform;
  std::vector<uint32_t> mesh_indices;  // Indices into loaded meshes
  std::vector<Node> children;

  /**
   * @brief Traverse the scene hierarchy depth-first
   *
   * @param func Function to call for each node (signature: void(const Node&, const XMFLOAT4X4&))
   * @param parent_matrix Parent world matrix (default: identity)
   *
   * Example usage:
   * @code
   * root_node.Traverse([](const Node& node, const DirectX::XMFLOAT4X4& world_matrix) {
   *   // Process node with its world transform
   * });
   * @endcode
   */
  template <typename Func>
  void Traverse(Func&& func,
    const DirectX::XMFLOAT4X4& parent_matrix = {
      1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}) const {
    // Calculate world matrix for this node
    DirectX::XMMATRIX parent_xml = DirectX::XMLoadFloat4x4(&parent_matrix);
    DirectX::XMMATRIX local_xml = DirectX::XMLoadFloat4x4(&transform.local_matrix);
    DirectX::XMMATRIX world_xml = parent_xml * local_xml;

    DirectX::XMFLOAT4X4 world_matrix;
    DirectX::XMStoreFloat4x4(&world_matrix, world_xml);

    // Call the user function
    func(*this, world_matrix);

    // Recursively traverse children
    for (const auto& child : children) {
      child.Traverse(std::forward<Func>(func), world_matrix);
    }
  }
};

/**
 * @brief Bone data for skeletal animation (reserved for future)
 *
 * This structure is reserved for future skeletal animation support.
 * Currently not used in the model loader.
 */
struct BoneData {
  std::string name;
  DirectX::XMFLOAT4X4 offset_matrix{
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};  // Offset matrix from mesh space to
                                                                                                      // bone space
  int32_t parent_index = -1;
};

/**
 * @brief Animation data for skeletal animation (reserved for future)
 *
 * This structure is reserved for future skeletal animation support.
 * Currently not used in the model loader.
 */
struct AnimationData {
  std::string name;
  float duration = 0.0f;
  float ticks_per_second = 0.0f;
  // TODO: Add keyframe data in future (position, rotation, scale keys)
};

}  // namespace Model
