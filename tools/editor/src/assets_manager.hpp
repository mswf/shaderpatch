#pragma once

#include "object_class.hpp"
#include "string_utilities.hpp"

#include <filesystem>

#include <boost/container/flat_map.hpp>

namespace sp::editor {

template<typename Type>
struct Basic_asset {
   Type asset;
   std::filesystem::path path;
};

template<typename Type>
using Assets_container =
   boost::container::flat_map<Ci_string, Basic_asset<Type>, std::less<>>;

class Assets_manager {
public:
   Assets_container<Object_class> object_classes;
};

inline void search_for_assets(Assets_manager& manager,
                              const std::filesystem::path& from);

}
