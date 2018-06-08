#pragma once

#include "assets_manager.hpp"
#include "object_instance.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sp::editor {

struct World_layer {
   std::vector<std::pair<std::string, Object_instance>> objects;
};

class Terrain {
};

class World {
public:
   World(std::filesystem::path path, std::string name);

   const std::filesystem::path& path() const noexcept;
   std::string_view name() const noexcept;

   World_layer* new_layer(std::string_view name);
   void delete_layer(std::string_view name);

   World_layer& layer(std::string_view name);

   World_layer& base_layer() noexcept;

   Terrain& terrain() noexcept;
   const Terrain& terrain() const noexcept;

   void update() noexcept;

   void drop_gpu_resources() noexcept;

private:
   std::filesystem::path _path;
   std::string _name;

   World_layer _base_layer;
};

World load_world(const std::filesystem::path& path, const Assets_manager& assets);

}
