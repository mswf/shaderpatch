#pragma once

#include "scene.hpp"

namespace sp::editor::msh {

// Flattens a scene down such that only the root node has children.
[[nodiscard]] Scene flatten_scene(const Scene& scene);

// Pretransform all segments in a scene and sets all node transforms to indentity transforms.
void pretransform_scene(Scene& scene) noexcept;

// Pretransform all segments in a node and sets the node's transform to the indentity transform.
void pretransform_node(Node& node) noexcept;

// Transforms a vector of segments using a matrix.
void transform_segments(std::vector<Segment>& segments,
                        const glm::mat4& transform) noexcept;

// Merge two segments, unique information is taken from `left`. Skinning information is dropped.
Segment merge_static_segments(const Segment& left, const Segment& right);

void regen_scene_bboxes(Scene& scene) noexcept;

[[nodiscard]] Aa_bbox build_scene_aabb(const Scene& scene) noexcept;

[[nodiscard]] Aa_bbox build_node_aabb(const Node& node) noexcept;

[[nodiscard]] Aa_bbox build_segments_aabb(const std::vector<Segment>& segments) noexcept;

[[nodiscard]] Aa_bbox build_cloths_aabb(const std::vector<Cloth>& cloths) noexcept;
}
