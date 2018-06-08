#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace sp::editor::geom {

auto spherical_unwrap(const std::vector<glm::vec3>& positions) -> std::vector<glm::vec2>
{
   std::vector<glm::vec2> uvs;
   uvs.reserve(positions.size());

   for (const auto& pos : positions) {
      const auto radius = glm::length(pos);

      uvs.emplace_back(glm::acos(pos.z / radius), glm::atan(pos.y / pos.x));
   }

   return uvs;
}

}
