#pragma once

#include "com_ptr.hpp"
#include "material.hpp"
#include "shared_array.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <d3d11_1.h>

namespace sp::editor {

class Mesh {
public:
   enum class Type { mesh, collision, collision_node, shadow };

   struct Vertex {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec3 tangent;
      glm::vec3 bitangent;
      glm::vec2 texcoords;
      glm::vec2 lightmap_texcoords;
   };

   static_assert(sizeof(Vertex) == 64);

   struct Meta_vertex {
      glm::vec3 position;
      glm::vec3 normal;
   };

   static_assert(sizeof(Meta_vertex) == 24);

   Type type;
   glm::mat3x4 transform;
   int material_index = -1;

   int index_buffer_offset = -1;
   int vertex_buffer_offset = -1;
   int vertex_buffer_stride = -1;

   Shared_array<std::array<std::uint16_t, 3>> indices;
   Shared_array<Vertex> vertices;
};

class Model_node {
public:
   std::vector<Mesh> meshes;
   std::vector<Model_node> nodes;

   glm::mat3x4 transform;
};

class Model {
public:
   std::vector<Material> materials;
   std::vector<Mesh> meshes;
   std::vector<Model_node> nodes;

   void drop_gpu_resources() noexcept;

   Com_ptr<ID3D11Buffer> vertex_buffer() noexcept;
   Com_ptr<ID3D11Buffer> index_buffer() noexcept;

private:
};

}
