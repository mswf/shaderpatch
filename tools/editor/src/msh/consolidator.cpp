#pragma once

#include "consolidator.hpp"
#include "iterator.hpp"
#include "operations.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std::literals;

namespace sp::editor::msh {

namespace {
void build_merge_list(const std::vector<const Segment*>& segments,
                      std::vector<std::vector<const Segment*>>& out_list);

auto get_merge_list(std::vector<const Segment*>::const_iterator first,
                    std::vector<const Segment*>::const_iterator last)
   -> std::pair<std::vector<const Segment*>, std::vector<const Segment*>::const_iterator>;

auto process_merge_lists(const std::vector<std::vector<const Segment*>>& merge_lists)
   -> std::vector<Segment>;
}

Scene Consolidator::consolidate(const Scene& scene)
{
   if (!std::all_of(std::cbegin(scene.root.children), std::cend(scene.root.children),
                    [](const Node& node) { return node.children.empty(); })) {
      throw std::invalid_argument{"Scene is not flat."};
   }

   *this = Consolidator{};

   Scene consolidated_scene{scene.scene_name, scene.bbox, scene.materials,
                            scene.root.shallow_copy()};

   sort_segments(scene);
   build_merge_lists();
   perform_merge(consolidated_scene);

   for (const auto& node : _other_nodes) {
      consolidated_scene.root.children.emplace_back(*node);
   }

   return consolidated_scene;
}

void Consolidator::sort_segments(const Scene& scene)
{
   for (const auto& node : Scene_iterator{scene.root}) {
      switch (node.type) {
      case Node_type::null:
      case Node_type::bone:
      case Node_type::collision_node:
         _other_nodes.emplace_back(&node);
         break;
      case Node_type::mesh: {
         auto lod = node.lod_suffix();

         for (auto& segment : node.segments) {
            _sorted_mesh_segments[std::pair{lod, segment.material_index}].emplace_back(
               &segment);
         }
      } break;
      case Node_type::shadow_mesh:
         for (auto& segment : node.segments) {
            _shadow_segments.emplace_back(&segment);
         }
      case Node_type::collision_mesh:
         for (auto& segment : node.segments) {
            _collision_segments.emplace_back(&segment);
         }
      }
   }
}

void Consolidator::build_merge_lists()
{
   for (const auto& group : _sorted_mesh_segments) {
      build_merge_list(group.second, _mesh_group_merge_lists[group.first]);
   }

   build_merge_list(_shadow_segments, _shadow_merge_lists);
   build_merge_list(_collision_segments, _collision_merge_lists);
}

void Consolidator::perform_merge(Scene& out_scene)
{
   perform_mesh_merge(out_scene);
   perform_shadow_merge(out_scene);
   perform_collision_merge(out_scene);
}

void Consolidator::perform_mesh_merge(Scene& out_scene)
{
   for (const auto& group : _mesh_group_merge_lists) {
      Node node;
      node.name = "mesh"s + group.first.first;
      node.type = Node_type::mesh;
      node.segments = process_merge_lists(group.second);

      out_scene.root.children.emplace_back(std::move(node));
   }
}

void Consolidator::perform_shadow_merge(Scene& out_scene)
{
   if (_shadow_merge_lists.empty()) return;

   Node node;
   node.name = "shadowvolume"s;
   node.type = Node_type::shadow_mesh;
   node.segments = process_merge_lists(_shadow_merge_lists);

   out_scene.root.children.emplace_back(std::move(node));
}

void Consolidator::perform_collision_merge(Scene& out_scene)
{
   if (_collision_merge_lists.empty()) return;

   Node node;
   node.name = "collision_mesh"s;
   node.type = Node_type::collision_mesh;
   node.segments = process_merge_lists(_collision_merge_lists);

   out_scene.root.children.emplace_back(std::move(node));
}

namespace {

void build_merge_list(const std::vector<const Segment*>& segments,
                      std::vector<std::vector<const Segment*>>& out_list)
{
   for (auto it = std::cbegin(segments); it != std::cend(segments); ++it) {
      std::tie(out_list.emplace_back(), it) = get_merge_list(it, std::cend(segments));
   }
}

auto get_merge_list(std::vector<const Segment*>::const_iterator first,
                    std::vector<const Segment*>::const_iterator last)
   -> std::pair<std::vector<const Segment*>, std::vector<const Segment*>::const_iterator>
{
   std::vector<const Segment*> merge_group;
   int vertex_count = 0;

   for (auto it = first; it != last; ++it) {
      if (vertex_count > Segment::max_vertex_count) return {merge_group, it};

      vertex_count += (*it)->positions.size();
      merge_group.emplace_back(*it);
   }

   return {merge_group, last};
}

auto process_merge_lists(const std::vector<std::vector<const Segment*>>& merge_lists)
   -> std::vector<Segment>
{
   std::vector<Segment> segments;

   for (const auto& list : merge_lists) {
      if (list.empty()) continue;

      Segment merged = *list.front();

      for (gsl::index i = 1; i < gsl::narrow_cast<gsl::index>(list.size()); ++i) {
         merge_static_segments(merged, *list[i]);
      }
   }

   return segments;
}

}
}
