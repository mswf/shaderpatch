
#include "scene.hpp"
#include "string_utilities.hpp"

#include <array>
#include <string_view>

using namespace std::literals;

namespace sp::editor::msh {

Node Node::shallow_copy() const noexcept
{
   return {name,     type,   hidden,   bbox,     transform,
           segments, cloths, bone_map, collision};
}

bool Node::secondary_lod() const noexcept
{
   const static std::array lod_suffices{"_lowres"sv, "_lowrez"sv, "_lod"sv,
                                        "_lod1"sv,   "_lod2"sv,   "_lod3"sv};

   for (auto suffix : lod_suffices) {
      if (ends_with(name, suffix)) return true;
   }

   return false;
}

std::string Node::lod_suffix() const noexcept
{
   if (!secondary_lod()) return ""s;

   return std::string{split_string_on_last(name, "_"sv)[1]};
}

}
