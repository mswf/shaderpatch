
#include "assets_manager.hpp"

namespace fs = std::filesystem;
using namespace std::literals;

namespace sp::editor {

void Assets_manager::search_for_assets(const std::filesystem::path& path)
{
   for (auto it = fs::recursive_directory_iterator{path};
        it != fs::recursive_directory_iterator{}; ++it) {
      if (!it->is_regular_file()) continue;

      if (auto ext = it->path().extension(); ext == ".odf"sv) {
         auto asset = std::make_shared<Odf_asset>(*this);
         asset->path = it->path();

         object_classes.emplace(make_ci_string(it->path().stem().u8string()),
                                std::move(asset));
      }
      else if (ext == ".msh"sv) {
         auto asset = std::make_shared<Model_asset>(*this);
         asset->path = it->path();

         model_files.emplace(make_ci_string(it->path().stem().u8string()),
                             std::move(asset));
      }
   }
}

}
