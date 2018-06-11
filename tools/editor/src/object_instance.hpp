#pragma once

#include "assets_manager.hpp"
#include "config_file.hpp"

#include <memory>
#include <utility>

#include <glm/glm.hpp>

namespace sp::editor {

class Object_instance {
public:
   Object_instance(const cfg::Node& object_node, const Assets_manager& assets);

   int team;
   odf::Properties properties;
   glm::mat4 transform{};

   auto odf_asset() const noexcept -> Odf_asset&;
   auto model_asset() const noexcept -> Model_asset&;

private:
   std::shared_ptr<Odf_asset> _definition;
   std::shared_ptr<Model_asset> _model;
};

}
