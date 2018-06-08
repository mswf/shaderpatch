#pragma once

#include "scene.hpp"

#include <map>

namespace sp::editor::msh {

// Class for merging geometry (mesh/shadow/collision mesh) nodes with each other
// using materials and LOD groups. Transforms, names of merged meshes, skinning
// information and cloths are ignored. Node AABBs are not rebuilt.
class Consolidator {
public:
   Scene consolidate(const Scene& scene);

private:
   void sort_segments(const Scene& scene);

   void build_merge_lists();

   void perform_merge(Scene& out_scene);

   void perform_mesh_merge(Scene& out_scene);

   void perform_shadow_merge(Scene& out_scene);

   void perform_collision_merge(Scene& out_scene);

   std::map<std::pair<std::string, std::int32_t>, std::vector<const Segment*>> _sorted_mesh_segments;
   std::vector<const Segment*> _shadow_segments;
   std::vector<const Segment*> _collision_segments;
   std::vector<const Node*> _other_nodes;

   std::map<std::pair<std::string, std::int32_t>, std::vector<std::vector<const Segment*>>> _mesh_group_merge_lists;
   std::vector<std::vector<const Segment*>> _shadow_merge_lists;
   std::vector<std::vector<const Segment*>> _collision_merge_lists;
};

}
