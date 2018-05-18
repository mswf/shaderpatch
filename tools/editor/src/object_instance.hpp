#pragma once

#include "object_class.hpp"

#include <memory>
#include <unordered_map>

namespace sp::editor {

class Object_instance {
public:
   using Key_value = Object_class::Key_value;

   auto properties() noexcept -> std::unordered_map<std::string, Key_value>&;
   auto properties() const noexcept
      -> const std::unordered_map<std::string, Key_value>&;

   auto object_class() const noexcept -> const std::shared_ptr<Object_class>&;

private:
};

}
