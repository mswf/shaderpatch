#pragma once

#include "object_instance.hpp"

#include <memory>
#include <string_view>

namespace sp::editor {

struct World_layer {
   std::vector<Object_instance> objects;
};

class Terrain {
};

class World {
public:
   World_layer* new_layer(std::string_view name);
   void delete_layer(std::string_view name);

   World_layer& layer(std::string_view name);

   Terrain& terrain() noexcept;
   const Terrain& terrain() const noexcept;

   void update() noexcept;

   void drop_gpu_resources() noexcept;

private:
};

}
