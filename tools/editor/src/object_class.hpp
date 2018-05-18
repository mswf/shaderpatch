#pragma once

#include <string>
#include <unordered_map>

namespace sp::editor {

class Object_class {
public:
   struct Key_value {
      std::string key;
      std::string value;
   };

   std::string_view class_label() const noexcept;
   void class_label(std::string_view label) const noexcept;

   std::string_view editor_geometry() const noexcept;
   void editor_geometry(std::string_view name) noexcept;

   float editor_scale() const noexcept;
   void editor_scale(float scale) noexcept;

   auto properties() noexcept -> std::unordered_map<std::string, Key_value>&;
   auto properties() const noexcept
      -> const std::unordered_map<std::string, Key_value>&;

   auto instance_properties() noexcept -> std::unordered_map<std::string, Key_value>&;
   auto instance_properties() const noexcept
      -> const std::unordered_map<std::string, Key_value>&;

private:
};

}
