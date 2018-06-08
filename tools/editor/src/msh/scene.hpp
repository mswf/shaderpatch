#pragma once

#include "../aa_bbox.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include <boost/container/small_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <gsl/gsl>

namespace sp::editor::msh {

enum class Node_type : std::uint32_t {
   null,
   bone,
   mesh,
   shadow_mesh,
   collision_mesh,
   collision_node
};

enum class Model_type : std::uint32_t {
   null = 0,
   skinned = 1,
   cloth = 2,
   bone = 3,
   fixed = 4,
   child_skinned = 5,
   shadow = 6
};

enum class Render_flags : std::uint8_t {
   normal = 0,
   emissive = 1,
   glow = 2,
   transparent = 4,
   doublesided = 8,
   hardedged = 16,
   perpixel = 32,
   additive = 64,
   specular = 128,
};

enum class Render_type : std::uint8_t {
   normal = 0,
   scrolling = 3,
   env_map = 6,
   animated = 7,
   glow = 11,
   unknown = 24,
   energy = 25,
   refraction = 22,
   bumpmap = 27
};

enum class Primitive_type : std::uint32_t {
   sphere = 0,
   cylinder = 2,
   cube = 4
};

enum class Cloth_collision_type : std::uint32_t {
   sphere = 0,
   cylinder = 1,
   cube = 2
};

struct Material {
   std::string name;

   glm::vec4 diffuse_colour{1.0f, 1.0f, 1.0f, 1.0f};
   glm::vec4 specular_colour{1.0f, 1.0f, 1.0f, 1.0f};
   glm::vec4 ambient_colour{1.0f, 1.0f, 1.0f, 1.0f};
   float specular_exponent{50.0f};
   std::array<std::string, 4> textures;

   Render_flags flags{};
   Render_type type{};

   std::array<std::int8_t, 2> params;
};

struct Skin_entry {
   std::int32_t bone;
   float weight;
};

struct Segment {
   std::int32_t material_index{0};
   std::vector<std::array<std::uint16_t, 3>> indices;
   std::vector<glm::vec3> positions;
   std::vector<glm::vec3> normals;
   std::vector<glm::u8vec4> colours;
   std::vector<glm::vec2> uv_coords;
   std::vector<std::array<Skin_entry, 4>> skin;

   std::vector<glm::vec3> shadow_positions;
   std::vector<std::array<std::uint16_t, 3>> shadow_edges;

   constexpr static auto max_vertex_count = 32767;
};

struct Cloth_collision {
   std::string name;
   std::string parent;

   Cloth_collision_type type = Cloth_collision_type::sphere;
   glm::vec3 size{0.0f, 0.0f, 0.0f};
};

struct Cloth {
   std::string texture;
   std::vector<glm::vec3> positions;
   std::vector<glm::vec2> uv_coords;
   std::vector<std::uint32_t> fixed_points;
   std::vector<std::string> fixed_weights;
   std::vector<std::array<std::uint32_t, 3>> indices;
   std::vector<std::array<std::uint16_t, 2>> stretch_constraints;
   std::vector<std::array<std::uint16_t, 2>> cross_constraints;
   std::vector<std::array<std::uint16_t, 2>> bend_constraints;

   std::vector<Cloth_collision> collision;
};

struct Collision_primitive {
   Primitive_type type = Primitive_type::cube;
   glm::vec3 size{0.0f, 0.0f, 0.0f};
};

struct Node {
   std::string name;
   Node_type type{};
   bool hidden{false};

   Aa_bbox bbox{};
   glm::mat4 transform{};

   std::vector<Segment> segments;
   std::vector<Cloth> cloths;
   std::vector<std::string> bone_map;

   std::optional<Collision_primitive> collision;

   std::vector<Node> children;

   Node shallow_copy() const noexcept;

   bool secondary_lod() const noexcept;
   std::string lod_suffix() const noexcept;
};

struct Scene {
   std::string scene_name;
   Aa_bbox bbox{};

   std::vector<Material> materials;
   Node root;
};

}
