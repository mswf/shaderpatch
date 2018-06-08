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

struct Mesh {
   struct Vertex {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 texcoords;
      glm::vec2 lightmap_texcoords;
   };

   static_assert(sizeof(Vertex) == 40);

   glm::mat4 transform;
   int material_index = -1;

   int index_buffer_offset = -1;
   int vertex_buffer_offset = -1;
   constexpr static int vertex_buffer_stride = sizeof(Vertex);
};

struct Meta_mesh {
   struct Vertex {
      glm::vec3 position;
      glm::vec3 normal;
   };

   static_assert(sizeof(Vertex) == 24);

   glm::mat4 transform;

   int index_buffer_offset = -1;
   int vertex_buffer_offset = -1;
   constexpr static int vertex_buffer_stride = sizeof(Vertex);
};

class Model {
public:
   Model();

   std::vector<Material> materials;
   std::vector<Mesh> meshes;

   void drop_gpu_resources() noexcept;

   Com_ptr<ID3D11Buffer> vertex_buffer() noexcept;
   Com_ptr<ID3D11Buffer> index_buffer() noexcept;

private:
};

}
