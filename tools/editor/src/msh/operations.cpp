#include "operations.hpp"
#include "iterator.hpp"

namespace sp::editor::msh {

Scene flatten_scene(const Scene& scene)
{
   msh::Scene flattened;
   flattened.materials = scene.materials;
   flattened.bbox = scene.bbox;
   flattened.scene_name = scene.scene_name;
   flattened.root = scene.root.shallow_copy();

   for (auto it = Scene_iterator{flattened.root}; it != end(it); ++it) {
      flattened.root.children.emplace_back(*it).transform = it.transform();
   }

   transform_segments(flattened.root.segments, flattened.root.transform);
   flattened.root.bbox = transform(flattened.root.bbox, flattened.root.transform);
   flattened.root.transform = glm::mat4{};

   return flattened;
}

void pretransform_scene(Scene& scene) noexcept
{
   for (auto& node : Scene_iterator{scene}) {
      switch (node.type) {
      case Node_type::null:
      case Node_type::bone:
      case Node_type::collision_node:
         continue;
      default:
         pretransform_node(node);
      }
   }
}

void pretransform_node(Node& node) noexcept
{
   Expects(node.children.empty());

   transform_segments(node.segments, node.transform);

   node.bbox = transform(node.bbox, node.transform);
   node.transform = glm::mat4{};
}

void transform_segments(std::vector<Segment>& segments, const glm::mat4& transform) noexcept
{
   for (auto& segment : segments) {
      for (auto& v : segment.positions) v = glm::vec4{v, 1.0f} * transform;

      for (glm::vec3& n : segment.normals) {
         n = glm::normalize(glm::vec4{n, 0.0f} * transform);
      }

      for (auto& v : segment.shadow_positions) {
         v = glm::vec4{v, 1.0f} * transform;
      }
   }
}

Segment merge_static_segments(const Segment& left, const Segment& right)
{
   Segment segment = left;
   segment.skin.clear();

   int index_offset = segment.positions.size();
   segment.indices.reserve(segment.indices.size() + right.indices.size());

   for (const auto& f : right.indices) {
      segment.indices.push_back(
         {gsl::narrow_cast<std::uint16_t>(f[0] + index_offset),
          gsl::narrow_cast<std::uint16_t>(f[1] + index_offset),
          gsl::narrow_cast<std::uint16_t>(f[2] + index_offset)});
   }

   segment.positions.insert(std::cend(segment.positions),
                            std::cbegin(right.positions), std::cend(right.positions));

   if (!segment.normals.empty()) {
      segment.normals.insert(std::cend(segment.normals),
                             std::cbegin(right.normals), std::cend(right.normals));
   }

   if (!segment.colours.empty()) {
      segment.colours.insert(std::cend(segment.colours),
                             std::cbegin(right.colours), std::cend(right.colours));
   }

   if (!segment.uv_coords.empty()) {
      segment.uv_coords.insert(std::cend(segment.uv_coords),
                               std::cbegin(right.uv_coords),
                               std::cend(right.uv_coords));
   }

   int shadow_edge_offset = segment.shadow_positions.size();

   for (const auto& e : right.shadow_edges) {
      segment.shadow_edges.push_back(
         {gsl::narrow_cast<std::uint16_t>(e[0] + shadow_edge_offset),
          gsl::narrow_cast<std::uint16_t>(e[1] + shadow_edge_offset),
          gsl::narrow_cast<std::uint16_t>(e[2] + shadow_edge_offset)});
   }

   if (!segment.shadow_positions.empty()) {
      segment.shadow_positions.insert(std::cend(segment.shadow_positions),
                                      std::cbegin(right.shadow_positions),
                                      std::cend(right.shadow_positions));
   }

   return segment;
}

void regen_scene_bboxes(Scene& scene) noexcept
{
   for (auto& node : Scene_iterator{scene}) {
      node.bbox = build_node_aabb(node);
   }

   scene.bbox = build_scene_aabb(scene);
}

Aa_bbox build_scene_aabb(const Scene& scene) noexcept
{
   Aa_bbox bbox{};

   for (auto it = Scene_iterator{scene}; it != end(it); ++it) {
      bbox = bbox_combine(bbox, transform(it->bbox, it.transform()));
   }

   return bbox;
}

Aa_bbox build_node_aabb(const Node& node) noexcept
{
   return bbox_combine(build_segments_aabb(node.segments),
                       build_cloths_aabb(node.cloths));
}

Aa_bbox build_segments_aabb(const std::vector<Segment>& segments) noexcept
{
   Aa_bbox bbox{};

   for (const auto& segment : segments) {
      bbox = bbox_combine(bbox, bbox_from_sequence(segment.positions));
      bbox = bbox_combine(bbox, bbox_from_sequence(segment.shadow_positions));
   }

   return bbox;
}

Aa_bbox build_cloths_aabb(const std::vector<Cloth>& cloths) noexcept
{
   Aa_bbox bbox{};

   for (const auto& cloth : cloths) {
      bbox = bbox_combine(bbox, bbox_from_sequence(cloth.positions));
   }

   return bbox;
}
}
