#include "mesh_factory.h"

namespace {

template <typename VertexType, typename IndexType>
MeshData BuildMeshData(const VertexType* verts, uint32_t vert_count, const IndexType* inds, uint32_t ind_count) {
  MeshData data;
  data.vertex_count = vert_count;
  data.vertices.resize(vert_count * sizeof(VertexType));
  memcpy(data.vertices.data(), verts, vert_count * sizeof(VertexType));
  data.indices.resize(ind_count);
  for (uint32_t i = 0; i < ind_count; ++i) {
    data.indices[i] = static_cast<uint32_t>(inds[i]);
  }
  return data;
}

}  // namespace

MeshData MeshFactory::CreateRectData() {
  SpriteVertex vertices[] = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {1, 1, 1, 1}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {1, 1, 1, 1}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}, {1, 1, 1, 1}},
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}, {1, 1, 1, 1}},
  };
  uint16_t indices[] = {0, 1, 2, 2, 3, 0};
  auto data = BuildMeshData(vertices, 4, indices, 6);
  data.layout = VertexDataLayout::Sprite;
  return data;
}

MeshData MeshFactory::CreateRoundedRectData(float corner_radius, uint32_t corner_segments, float aspect_ratio) {
  constexpr float PI = Math::Pi;
  constexpr float HALF = 0.5f;

  float r_x = (aspect_ratio >= 1.0f) ? corner_radius / aspect_ratio : corner_radius;
  float r_y = (aspect_ratio >= 1.0f) ? corner_radius : corner_radius * aspect_ratio;

  std::vector<SpriteVertex> vertices;
  std::vector<uint16_t> indices;

  auto add_vertex = [&](float x, float y) -> uint16_t {
    float u = x + HALF;
    float v = HALF - y;
    vertices.push_back({{x, y, 0.0f}, {u, v}, {1, 1, 1, 1}});
    return static_cast<uint16_t>(vertices.size() - 1);
  };

  uint16_t center = add_vertex(0.0f, 0.0f);

  float inner_x = HALF - r_x;
  float inner_y = HALF - r_y;

  struct CornerDef {
    float cx, cy, start_angle;
  };
  CornerDef corners[] = {
    {+inner_x, +inner_y, 0.0f},
    {-inner_x, +inner_y, PI * 0.5f},
    {-inner_x, -inner_y, PI},
    {+inner_x, -inner_y, PI * 1.5f},
  };

  std::vector<uint16_t> perimeter;
  for (auto& [cx, cy, start] : corners) {
    for (uint32_t j = 0; j <= corner_segments; ++j) {
      float angle = start + static_cast<float>(j) / corner_segments * (PI * 0.5f);
      float x = cx + r_x * cosf(angle);
      float y = cy + r_y * sinf(angle);
      uint16_t idx = add_vertex(x, y);
      if (j == 0 && !perimeter.empty() && perimeter.back() == idx) continue;
      perimeter.push_back(idx);
    }
  }

  for (size_t i = 0; i < perimeter.size(); ++i) {
    size_t next = (i + 1) % perimeter.size();
    indices.push_back(center);
    indices.push_back(perimeter[i]);
    indices.push_back(perimeter[next]);
  }

  auto data = BuildMeshData(vertices.data(), static_cast<uint32_t>(vertices.size()), indices.data(), static_cast<uint32_t>(indices.size()));
  data.layout = VertexDataLayout::Sprite;
  return data;
}

MeshData MeshFactory::CreateQuadData() {
  ModelVertex vertices[] = {
    {{-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
  };
  uint16_t indices[] = {0, 1, 2, 2, 3, 0};
  return BuildMeshData(vertices, 4, indices, 6);
}

/**
 * CreateCube Corner
 *    6--------7      Y+
 *   /|       /|      |
 *  4--------5 |      +-- X+
 *  | 2------|-3     /
 *  |/       |/     Z+
 *  0--------1
 */
MeshData MeshFactory::CreateCubeData() {
  // Tangent = U direction in world space, w = bitangent sign
  // bitangent = cross(normal, tangent.xyz) * tangent.w
  ModelVertex vertices[] = {
    // Front face (Z+): U=(1,0,0), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
    {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    // Back face (Z-): U=(-1,0,0), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
    {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
    {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
    {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
    {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
    // Top face (Y+): U=(1,0,0), cross(N,T)=(0,0,-1), actual_B=(0,0,-1) → w=1
    {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    // Bottom face (Y-): U=(1,0,0), cross(N,T)=(0,0,1), actual_B=(0,0,1) → w=1
    {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
    // Right face (X+): U=(0,0,-1), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
    {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
    {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
    {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
    {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
    // Left face (X-): U=(0,0,1), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
    {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
    {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
    {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
    {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
  };
  uint16_t indices[] = {
    0,
    1,
    2,
    2,
    3,
    0,  // Front
    4,
    5,
    6,
    6,
    7,
    4,  // Back
    8,
    9,
    10,
    10,
    11,
    8,  // Top
    12,
    13,
    14,
    14,
    15,
    12,  // Bottom
    16,
    17,
    18,
    18,
    19,
    16,  // Right
    20,
    21,
    22,
    22,
    23,
    20,  // Left
  };
  return BuildMeshData(vertices, 24, indices, 36);
}

/**
 * CreateCube Corner
 *    6--------7      Y+
 *   /|       /|      |
 *  4--------5 |      +-- X+
 *  | 2------|-3     /
 *  |/       |/     Z+
 *  0--------1
 */
MeshData MeshFactory::CreateCubeData(const CubeCornerColors& corner_colors) {
  // Corner-to-vertex mapping per face (matches CreateCube(device, mesh, colors) in mesh_factory.h)
  // Front(0,1,5,4) Back(3,2,6,7) Top(4,5,7,6) Bottom(2,3,1,0) Right(1,3,7,5) Left(2,0,4,6)
  constexpr int CORNER_MAP[24] = {
    0,
    1,
    5,
    4,  // Front face
    3,
    2,
    6,
    7,  // Back face
    4,
    5,
    7,
    6,  // Top face
    2,
    3,
    1,
    0,  // Bottom face
    1,
    3,
    7,
    5,  // Right face
    2,
    0,
    4,
    6,  // Left face
  };
  MeshData data = CreateCubeData();
  auto* verts = reinterpret_cast<ModelVertex*>(data.vertices.data());
  for (int i = 0; i < 24; ++i) {
    verts[i].color = corner_colors[CORNER_MAP[i]];
  }
  return data;
}

MeshData MeshFactory::CreatePlaneData(uint32_t subdivisions_x, uint32_t subdivisions_z) {
  uint32_t vertex_count_x = subdivisions_x + 1;
  uint32_t vertex_count_z = subdivisions_z + 1;

  std::vector<ModelVertex> vertices;
  vertices.reserve(vertex_count_x * vertex_count_z);

  for (uint32_t z = 0; z < vertex_count_z; ++z) {
    for (uint32_t x = 0; x < vertex_count_x; ++x) {
      float u = static_cast<float>(x) / subdivisions_x;
      float v = static_cast<float>(z) / subdivisions_z;
      vertices.push_back({{u - 0.5f, 0.0f, v - 0.5f}, {0, 1, 0}, {u, v}, {1, 1, 1, 1}, {1, 0, 0, -1}});
    }
  }

  std::vector<uint16_t> indices;
  indices.reserve(subdivisions_x * subdivisions_z * 6);
  for (uint32_t z = 0; z < subdivisions_z; ++z) {
    for (uint32_t x = 0; x < subdivisions_x; ++x) {
      uint16_t tl = static_cast<uint16_t>(z * vertex_count_x + x);
      uint16_t tr = static_cast<uint16_t>(tl + 1);
      uint16_t bl = static_cast<uint16_t>((z + 1) * vertex_count_x + x);
      uint16_t br = static_cast<uint16_t>(bl + 1);
      indices.push_back(tl);
      indices.push_back(bl);
      indices.push_back(tr);
      indices.push_back(tr);
      indices.push_back(bl);
      indices.push_back(br);
    }
  }

  return BuildMeshData(vertices.data(), static_cast<uint32_t>(vertices.size()), indices.data(), static_cast<uint32_t>(indices.size()));
}

MeshData MeshFactory::CreateSphereData(uint32_t segments, uint32_t rings) {
  constexpr float PI = Math::Pi;
  constexpr float RADIUS = 0.5f;

  std::vector<ModelVertex> vertices;
  vertices.reserve((rings + 1) * (segments + 1));

  for (uint32_t ring = 0; ring <= rings; ++ring) {
    float v = static_cast<float>(ring) / rings;
    float theta = v * PI;
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    for (uint32_t seg = 0; seg <= segments; ++seg) {
      float u = static_cast<float>(seg) / segments;
      float phi = u * 2.0f * PI;
      float sin_phi = sinf(phi);
      float cos_phi = cosf(phi);

      float nx = sin_theta * cos_phi;
      float ny = cos_theta;
      float nz = sin_theta * sin_phi;

      float tx = -sin_phi;
      float tz = cos_phi;
      float len = sqrtf(tx * tx + tz * tz);
      if (len > 0.0f) {
        tx /= len;
        tz /= len;
      }

      vertices.push_back({
        {nx * RADIUS, ny * RADIUS, nz * RADIUS},
        {nx, ny, nz},
        {u, v},
        {1, 1, 1, 1},
        {tx, 0, tz, 1},
      });
    }
  }

  std::vector<uint16_t> indices;
  indices.reserve(rings * segments * 6);
  for (uint32_t ring = 0; ring < rings; ++ring) {
    for (uint32_t seg = 0; seg < segments; ++seg) {
      uint16_t cur = static_cast<uint16_t>(ring * (segments + 1) + seg);
      uint16_t next = static_cast<uint16_t>(cur + 1);
      uint16_t below = static_cast<uint16_t>(cur + segments + 1);
      uint16_t below_next = static_cast<uint16_t>(below + 1);
      indices.push_back(cur);
      indices.push_back(next);
      indices.push_back(below);
      indices.push_back(next);
      indices.push_back(below_next);
      indices.push_back(below);
    }
  }

  return BuildMeshData(vertices.data(), static_cast<uint32_t>(vertices.size()), indices.data(), static_cast<uint32_t>(indices.size()));
}

MeshData MeshFactory::CreateCylinderData(uint32_t segments) {
  constexpr float PI = Math::Pi;
  constexpr float RADIUS = 0.5f;
  constexpr float HALF_HEIGHT = 0.5f;

  std::vector<ModelVertex> vertices;
  vertices.reserve((segments + 1) * 2);

  for (uint32_t i = 0; i <= segments; ++i) {
    float u = static_cast<float>(i) / segments;
    float angle = u * 2.0f * PI;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    vertices.push_back({
      {cos_a * RADIUS, -HALF_HEIGHT, sin_a * RADIUS},
      {cos_a, 0, sin_a},
      {0, u},
      {1, 1, 1, 1},
      {-sin_a, 0, cos_a, 1},
    });
    vertices.push_back({
      {cos_a * RADIUS, HALF_HEIGHT, sin_a * RADIUS},
      {cos_a, 0, sin_a},
      {1, u},
      {1, 1, 1, 1},
      {-sin_a, 0, cos_a, 1},
    });
  }

  std::vector<uint16_t> indices;
  indices.reserve(segments * 6);
  for (uint32_t i = 0; i < segments; ++i) {
    uint16_t bl = static_cast<uint16_t>(i * 2);
    uint16_t tl = static_cast<uint16_t>(i * 2 + 1);
    uint16_t br = static_cast<uint16_t>((i + 1) * 2);
    uint16_t tr = static_cast<uint16_t>((i + 1) * 2 + 1);
    indices.push_back(bl);
    indices.push_back(br);
    indices.push_back(tl);
    indices.push_back(tl);
    indices.push_back(br);
    indices.push_back(tr);
  }

  return BuildMeshData(vertices.data(), static_cast<uint32_t>(vertices.size()), indices.data(), static_cast<uint32_t>(indices.size()));
}
