#include "model.hpp"
#include "algorithm.hpp"
#include "msh/iterator.hpp"

#include <gsl/gsl>

namespace sp::editor {

namespace {
std::size_t calculate_scene_buffer_size(const msh::Scene& scene) noexcept;

std::size_t get_node_vertex_size(const msh::Node& node) noexcept;
}

Model::Model(const msh::Scene& scene)
   : _buffer{calculate_scene_buffer_size(scene)}
{
   int buffer_offset = 0;

   for (const auto& node : msh::Scene_iterator{scene}) {
      for (const auto& segment : node.segments) {
         switch (node.type) {
         case msh::Node_type::mesh:
            buffer_offset = add_mesh_segment(node, segment, buffer_offset);
            break;
         case msh::Node_type::collision_mesh:
         case msh::Node_type::collision_node:
            buffer_offset = add_collision_segment(node, segment, buffer_offset);
            break;
         default:
            continue;
         }
      }
   }
}

int Model::add_mesh_segment(const msh::Node& owning_node,
                            const msh::Segment& segment, int buffer_offset)
{
   Mesh mesh;
   mesh.transform = owning_node.transform;
   mesh.index_buffer_offset = buffer_offset;

   buffer_offset +=
      range_memcpy(_buffer.to_span().subspan(buffer_offset, _buffer.size() - buffer_offset),
                   segment.indices);

   mesh.vertex_buffer_offset = buffer_offset;

   for (gsl::index i = 0;
        i < gsl::narrow_cast<gsl::index>(segment.positions.size()); ++i) {
      new (&_buffer[buffer_offset])
         Mesh::Vertex{segment.positions[i], segment.normals[i],
                      segment.uv_coords[i], segment.uv_coords[i]};

      buffer_offset += sizeof(Mesh::Vertex);
   }

   meshes.emplace_back(std::move(mesh));

   return buffer_offset;
}

int Model::add_collision_segment(const msh::Node& owning_node,
                                 const msh::Segment& segment, int buffer_offset)
{
   Meta_mesh mesh;
   mesh.transform = owning_node.transform;
   mesh.index_buffer_offset = buffer_offset;

   buffer_offset +=
      range_memcpy(_buffer.to_span().subspan(buffer_offset, _buffer.size() - buffer_offset),
                   segment.indices);

   mesh.vertex_buffer_offset = buffer_offset;

   for (gsl::index i = 0;
        i < gsl::narrow_cast<gsl::index>(segment.positions.size()); ++i) {
      new (&_buffer[buffer_offset])
         Meta_mesh::Vertex{segment.positions[i], segment.normals[i]};

      buffer_offset += sizeof(Meta_mesh::Vertex);
   }

   collision_meshes.emplace_back(std::move(mesh));

   return buffer_offset;
}

namespace {

std::size_t calculate_scene_buffer_size(const msh::Scene& scene) noexcept
{
   std::size_t size{};

   for (const auto& node : msh::Scene_iterator{scene}) {
      const auto vertex_size = get_node_vertex_size(node);

      for (const auto& segment : node.segments) {
         size += vertex_size * segment.positions.size();
         size += sizeof(std::array<std::uint16_t, 3>) * segment.indices.size();
      }
   }

   return size;
}

std::size_t get_node_vertex_size(const msh::Node& node) noexcept
{
   switch (node.type) {
   case msh::Node_type::null:
   case msh::Node_type::bone:
   case msh::Node_type::collision_node:
      return sizeof(Meta_mesh::Vertex);
   case msh::Node_type::mesh:
   case msh::Node_type::shadow_mesh:
   case msh::Node_type::collision_mesh:
      return sizeof(Mesh::Vertex);
   }

   return 0;
}
}

}
