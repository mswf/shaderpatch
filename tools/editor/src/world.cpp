
#include "world.hpp"
#include "compose_exception.hpp"
#include "config_file.hpp"
#include "exceptions.hpp"

#include <fstream>

namespace fs = std::filesystem;
using namespace std::literals;

namespace sp::editor {

World::World(std::filesystem::path path, std::string name)
   : _path{std::move(path)}, _name{std::move(name)}
{
}

World_layer& World::base_layer() noexcept
{
   return _base_layer;
}

namespace {

auto read_layer_index(const fs::path& path) -> std::vector<std::string>
{
   std::ifstream file{};
   file.exceptions(std::ios::badbit | std::ios::failbit);
   file.open(path);

   cfg::Node index;
   file >> index;

   std::vector<std::string> layers;

   for (auto& child : index) {
      if (auto& node = child.second; child.first == "Version"sv) {
         if (node.get_value<int>() != 1) {
            throw compose_exception<Parse_error>(
               "Unexpected Version value in layer index '",
               node.get_value<std::string>(), "'."sv);
         }
      }
      else if (child.first == "NextID"sv) {
         continue;
      }
      else if (child.first == "Layer"sv) {
         auto name = node.get_value<std::string>(0);

         if (name == "[Base]"sv) continue;

         layers.emplace_back(std::move(name));
      }
   }

   return layers;
}

auto read_objects(const fs::path& path, const Assets_manager& assets)
   -> std::vector<std::pair<std::string, Object_instance>>
{
   std::ifstream file{};
   file.exceptions(std::ios::badbit | std::ios::failbit);
   file.open(path);

   cfg::Node world;
   file >> world;

   std::vector<std::pair<std::string, Object_instance>> objects;

   for (auto& child : world) {
      if (child.first != "Object"sv) continue;

      objects.emplace_back(child.second.get_value<std::string>(0),
                           Object_instance{child.second, assets});
   }

   return objects;
}

}

World load_world(const fs::path& path, const Assets_manager& assets)
{
   const auto name = path.stem().u8string();

   World world{path.parent_path(), name};

   auto layers = read_layer_index(fs::path{path}.replace_extension(".ldx"sv));

   world.base_layer().objects = read_objects(path, assets);

   return world;
}

}
