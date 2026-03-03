#ifndef MESH_DESCRIPTOR_HLSLI
#define MESH_DESCRIPTOR_HLSLI

struct MeshDescriptor {
  uint vertex_offset;
  uint vertex_count;
  uint index_offset;
  uint index_count;
};

StructuredBuffer<MeshDescriptor> g_MeshDescriptors : register(t0, space4);

#endif
