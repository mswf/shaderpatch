
#include "project.hpp"

namespace sp::editor {

namespace fs = std::filesystem;
using namespace std::literals;

Project::Project(fs::path path) : _project_root{std::move(path)}
{
   fs::current_path(_project_root);

   _assets.search_for_assets(_project_root);

   for (auto& entry : fs::recursive_directory_iterator{path / "Worlds"sv}) {
      if (!entry.is_regular_file() || entry.path().extension() != ".wld"sv) {
         continue;
      }

      _worlds.emplace_back(load_world(entry, _assets));
   }
}

}
