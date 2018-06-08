#pragma once

#include "assets_manager.hpp"
#include "world.hpp"

#include <filesystem>
#include <vector>

namespace sp::editor {

class Project {
public:
   Project() = default;

   explicit Project(std::filesystem::path path);

private:
   Assets_manager _assets;

   World* _active_world;

   std::vector<World> _worlds;

   std::filesystem::path _project_root;
};

}
