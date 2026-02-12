#pragma once

namespace Model {

template <typename T>
concept VertexWithPosition = requires(T vertex) {
    { vertex.position };
};

template <typename T>
concept VertexWithNormal = requires(T vertex) {
    { vertex.normal };
};

template <typename T>
concept VertexWithTangent = requires(T vertex) {
    { vertex.tangent };
};

template <typename T>
concept VertexWithTexcoord = requires(T vertex) {
    { vertex.texcoord };
};

}  // namespace Model
