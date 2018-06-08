
#include "object_instance.hpp"
#include "exceptions.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std::literals;

namespace sp::editor {

namespace {

bool is_well_formed_node(const cfg::Node& object_node)
{
   if (object_node.values_count() != 3) return false;
   if (object_node.size() < 5) return false;

   const auto end = std::cend(object_node);

   if (auto it = object_node.find("ChildRotation"sv);
       it == end || it->second.values_count() != 4) {

      return false;
   }

   if (auto it = object_node.find("ChildPosition"sv);
       it == end || it->second.values_count() != 3) {

      return false;
   }

   if (auto it = object_node.find("SeqNo"sv);
       it == end || it->second.values_count() != 1) {

      return false;
   }

   if (auto it = object_node.find("Team"sv);
       it == end || it->second.values_count() != 1) {

      return false;
   }

   if (auto it = object_node.find("NetworkId"sv);
       it == end || it->second.values_count() != 1) {

      return false;
   }

   return true;
}

glm::mat4 read_transform(const cfg::Node& object_node)
{
   auto& rotation_node = object_node.at("ChildRotation"sv);
   glm::quat rotation{rotation_node.value(0), rotation_node.value(1),
                      rotation_node.value(2), rotation_node.value(3)};

   glm::mat4 matrix = static_cast<glm::mat3>(rotation);

   auto position_node = object_node.at("ChildPosition"sv);

   matrix[0].w = position_node.value(0);
   matrix[1].w = position_node.value(1);
   matrix[2].w = position_node.value(2);

   return matrix;
}

}

Object_instance::Object_instance(const cfg::Node& object_node,
                                 const Assets_manager& assets)
{
   Ci_string classname = make_ci_string(object_node.get_value<std::string>(1));

   if (!is_well_formed_node(object_node)) {
      throw Asset_load_error{"Badly formed object node."s};
   }

   transform = read_transform(object_node);
   team = object_node.at("Team"sv).value();

   _definition =
      assets.object_classes.at(make_ci_string(std::string{object_node.value(1)}));

   properties = _definition->asset().instance_properties;

   for (auto& prop : properties) {
      if (auto it = object_node.find(prop.first); it != std::cend(object_node)) {
         prop.second.value = it->second.value(0);

         if (!it->second.trailing_comment().empty()) {
            prop.second.trailing_comment = it->second.trailing_comment();
         }

         if (!it->second.comments().empty()) {
            prop.second.comments = it->second.comments();
         }
      }
   }
}

auto Object_instance::definition() const noexcept -> const std::shared_ptr<Odf_asset>&
{
   return _definition;
}

}
