#pragma once

#include "unwrap.hpp"

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>
#include <gsl/gsl>

namespace sp::editor::geom {

// Returns [positions, normals, uvs, faces] packed into a tuple.
auto create_icosphere(float radius, int subdivisions = 2)
   -> std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>,
                 std::vector<glm::vec2>, std::vector<std::array<std::uint16_t, 3>>>;

// Returns [positions, normals, uvs, faces] packed into a tuple.
auto create_cylinder(float radius, float height, int sections = 32)
   -> std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>,
                 std::vector<glm::vec2>, std::vector<std::array<std::uint16_t, 3>>>;

}
