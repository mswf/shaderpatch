#pragma once

#include "scene.hpp"

namespace sp::editor::msh {

template<typename Type>
class Scene_iterator {
public:
   using difference_type = std::ptrdiff_t;
   using value_type = std::conditional_t<std::is_const_v<Type>, const Node, Node>;
   using pointer = value_type*;
   using reference = value_type&;
   using iterator_category = std::input_iterator_tag;

   Scene_iterator() = default;

   Scene_iterator(Type& scene_or_node) noexcept
   {
      if constexpr (std::is_same_v<std::decay_t<Type>, Scene>) {
         _stack.emplace(&scene_or_node.root, -1, scene_or_node.root.transform);
      }
      else {
         for (auto& node : scene_or_node.children) {
            _stack.emplace(&node, -1, scene_or_node.transform * node.transform);
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

   auto operator++() -> Scene_iterator&
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

   void swap(Scene_iterator& other) noexcept
   {
      std::swap(this->_stack, other._stack);
   }

   auto transform() const noexcept -> const glm::mat4&
   {
      return _stack.top().transform;
   }

   template<typename Type>
   friend bool operator==(const Scene_iterator<Type>& left, std::nullptr_t) noexcept;

private:
   struct Level {
      Level(value_type* node, gsl::index index, glm::mat4 transform = glm::mat4{}) noexcept
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
inline auto begin(const Scene_iterator<Type>& iter) noexcept -> Scene_iterator<Type>
{
   return iter;
}

template<typename Type>
inline auto end(const Scene_iterator<Type>&) noexcept
{
   return nullptr;
}

template<typename Type>
inline bool operator==(const Scene_iterator<Type>& it, std::nullptr_t) noexcept
{
   return it._stack.empty();
}

template<typename Type>
inline bool operator==(std::nullptr_t, const Scene_iterator<Type>& it) noexcept
{
   return it == nullptr;
}

template<typename Type>
inline bool operator!=(const Scene_iterator<Type>& it, std::nullptr_t) noexcept
{
   return !(it == nullptr);
}

template<typename Type>
inline bool operator!=(std::nullptr_t, const Scene_iterator<Type>& left) noexcept
{
   return it != nullptr;
}

inline Node* find_node(Scene& scene, std::string_view node_name) noexcept
{
   for (auto& node : Scene_iterator{scene}) {
      if (node.name == node_name) return &node;
   }

   return nullptr;
}

inline const Node* find_node(const Scene& scene, std::string_view node_name) noexcept
{
   for (const auto& node : Scene_iterator{scene}) {
      if (node.name == node_name) return &node;
   }

   return nullptr;
}

}
