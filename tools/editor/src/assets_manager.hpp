#pragma once

#include "asset_loaders.hpp"
#include "compose_exception.hpp"
#include "exceptions.hpp"
#include "model.hpp"
#include "odf.hpp"
#include "string_utilities.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include <boost/container/flat_map.hpp>

namespace sp::editor {

class Assets_manager;

template<typename Type>
class Basic_asset {
public:
   Basic_asset(Assets_manager& owner) : _owner{owner} {}

   Type& asset()
   {
      if (!_asset) load();

      return *_asset;
   }

   Type* peak_asset() noexcept
   {
      return _asset ? &_asset.value() : nullptr;
   }

   const Type* peak_asset() const noexcept
   {
      return _asset ? &_asset.value() : nullptr;
   }

   std::filesystem::path path;

private:
   void load()
   {
      try {
         _asset = Asset_loader<Type>{}.load(path, _owner);
      }
      catch (std::exception& e) {
         throw compose_exception<Asset_load_error>("Exception occured while loading "sv,
                                                   path, " message was: "sv,
                                                   e.what());
      }
   }

   Assets_manager& _owner;
   std::optional<Type> _asset;
};

template<typename Type>
using Assets_container =
   boost::container::flat_map<Ci_string, std::shared_ptr<Basic_asset<Type>>, std::less<>>;

class Assets_manager {
public:
   void search_for_assets(const std::filesystem::path& path);

   Assets_container<odf::Definition> object_classes;
   Assets_container<Model> model_files;
};

using Odf_asset = Basic_asset<odf::Definition>;
using Model_asset = Basic_asset<Model>;

}
