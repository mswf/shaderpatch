#pragma once

#include "aa_bbox.hpp"

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
   collision_node,
   cloth
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
   std::int32_t material_index{-1};
   std::vector<std::uint16_t> strips;
   std::vector<glm::vec3> positions;
   std::vector<glm::vec3> normals;
   std::vector<glm::u8vec4> colours;
   std::vector<glm::vec2> texture_coords;
   std::vector<std::array<Skin_entry, 4>> skin;

   std::vector<glm::vec3> shadow_positions;
   std::vector<glm::u16vec3> shadow_edges;
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
   std::vector<glm::vec2> texture_coords;
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

   bool secondary_lod() const noexcept;
};

struct Model {
   std::string scene_name;
   Aa_bbox bbox{};

   std::vector<Material> materials;
   std::vector<Node> root_nodes;
};

Model load_model(const std::filesystem::path& path);

template<typename Type>
class Recursive_model_iterator {
public:
   using difference_type = std::ptrdiff_t;
   using value_type = std::conditional_t<std::is_const_v<Type>, const Node, Node>;
   using pointer = value_type*;
   using reference = value_type&;
   using iterator_category = std::input_iterator_tag;

   Recursive_model_iterator() = default;

   template<typename = std::void_t<decltype(Type::root_nodes)>>
   Recursive_model_iterator(Type& model) noexcept
   {
      if constexpr (std::is_same_v<std::decay_t<Type>, Model>) {
         for (auto& node : model.root_nodes) _stack.emplace(&node, -1);
      }
      else {
         for (auto& node : model.children) {
            _stack.emplace(&node, -1, node.transform);
         }
      }
   }

   auto operator*() const noexcept -> reference
   {
      auto& top = _stack.top();

      if (top.index == -1) return *top.node;

      return top.node->children[top.index];
   }

   auto operator-> () const noexcept -> pointer
   {
      return &(*(*this));
   }

   auto operator++() -> Recursive_model_iterator&
   {
      if (_stack.empty()) return *this;

      auto& top = _stack.top();

      top.index += 1;

      if (top.index >= gsl::narrow_cast<gsl::index>(top.node->children.size())) {
         _stack.pop();

         return ++(*this);
      }

      if (auto& child = top.node->children[top.index]; !child.children.empty()) {
         _stack.emplace(&child, -1, _stack.top().transform * child.transform);
      }

      return *this;
   }

   void swap(Recursive_model_iterator& other) noexcept
   {
      std::swap(this->_stack, other._stack);
   }

   auto transform() const noexcept -> const glm::mat4&
   {
      return _stack.top().transform;
   }

   template<typename Type>
   friend bool operator==(const Recursive_model_iterator<Type>& left,
                          std::nullptr_t) noexcept;

private:
   struct Level {
      Level(value_type* node, gsl::index index,
            glm::mat3x4 transform = glm::mat3x4{}) noexcept
         : node{node}, index{index}, transform{transform}
      {
      }

      value_type* node;
      gsl::index index;
      glm::mat4 transform;
   };

   std::stack<Level, boost::container::small_vector<Level, 8>> _stack;
};

template<typename Type>
inline auto begin(const Recursive_model_iterator<Type>& iter) noexcept
   -> Recursive_model_iterator<Type>
{
   return iter;
}

template<typename Type>
inline auto end(const Recursive_model_iterator<Type>&) noexcept
{
   return nullptr;
}

template<typename Type>
inline bool operator==(const Recursive_model_iterator<Type>& it, std::nullptr_t) noexcept
{
   return it._stack.empty();
}

template<typename Type>
inline bool operator==(std::nullptr_t, const Recursive_model_iterator<Type>& it) noexcept
{
   return it == nullptr;
}

template<typename Type>
inline bool operator!=(const Recursive_model_iterator<Type>& it, std::nullptr_t) noexcept
{
   return !(it == nullptr);
}

template<typename Type>
inline bool operator!=(std::nullptr_t, const Recursive_model_iterator<Type>& left) noexcept
{
   return it != nullptr;
}

inline Node* find_node(Model& model, std::string_view node_name) noexcept
{
   for (auto& node : Recursive_model_iterator{model}) {
      if (node.name == node_name) return &node;
   }

   return nullptr;
}

inline const Node* find_node(const Model& model, std::string_view node_name) noexcept
{
   for (const auto& node : Recursive_model_iterator{model}) {
      if (node.name == node_name) return &node;
   }

   return nullptr;
}

}
