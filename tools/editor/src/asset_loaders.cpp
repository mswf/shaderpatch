#include "asset_loaders.hpp"
#include "assets_manager.hpp"
#include "msh/consolidator.hpp"
#include "msh/load.hpp"
#include "msh/operations.hpp"

#include <memory>

namespace sp::editor {

odf::Definition Asset_loader<odf::Definition>::load(const std::filesystem::path& path,
                                                    Assets_manager&)
{
   odf::Definition definition;

   auto file = safe_ifile_open(path);
   file >> definition;

   return definition;
}

Model Asset_loader<Model>::load(const std::filesystem::path& path, Assets_manager& manager)
{
   auto scene = msh::load_from_file(path);
   scene = msh::flatten_scene(scene);
   msh::pretransform_scene(scene);
   scene = msh::Consolidator{}.consolidate(scene);

   return Model{scene};
}

}
