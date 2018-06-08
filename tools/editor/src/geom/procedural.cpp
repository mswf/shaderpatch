
#include "procedural.hpp"

#include <algorithm>
#include <map>

namespace sp::editor::geom {

// Returns a [positions, normals, uvs, faces] packed into a tuple.
auto create_icosphere(float radius, int subdivisions)
   -> std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>,
                 std::vector<glm::vec2>, std::vector<std::array<std::uint16_t, 3>>>
{
   subdivisions = std::clamp(subdivisions, 0, 6);

   std::vector<glm::vec3>
      positions{{-0.526f, 0.851f, 0.000f},  {0.526f, 0.851f, 0.000f},
                {-0.526f, -0.851f, 0.000f}, {0.526f, -0.851f, 0.000f},
                {0.000f, -0.526f, 0.851f},  {0.000f, 0.526f, 0.851f},
                {0.000f, -0.526f, -0.851f}, {0.000f, 0.526f, -0.851f},
                {0.851f, 0.000f, -0.526f},  {0.851f, 0.000f, 0.526f},
                {-0.851f, 0.000f, -0.526f}, {-0.851f, 0.000f, 0.526f}};

   std::vector<std::array<std::uint16_t, 3>>
      faces{{0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
            {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
            {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
            {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1}};

   std::map<std::pair<std::uint16_t, std::uint16_t>, std::uint16_t> edge_cache;

   const auto subdivide_edge = [&](std::uint16_t v1, std::uint16_t v2) {
      if (auto cached = edge_cache.find(std::minmax(v1, v2));
          cached != std::cend(edge_cache)) {
         return cached->second;
      }

      auto v3 = gsl::narrow_cast<std::uint16_t>(positions.size());

      positions.emplace_back(glm::normalize((positions[v1] + positions[v2]) * 0.5f));

      return edge_cache[std::minmax(v1, v2)] = v3;
   };

   const auto subdivide = [&] {
      std::vector<std::array<std::uint16_t, 3>> subd_tris;
      subd_tris.reserve(faces.size() * 4);

      for (auto& f : faces) {
         const auto v4 = subdivide_edge(f[0], f[1]);
         const auto v5 = subdivide_edge(f[1], f[2]);
         const auto v6 = subdivide_edge(f[2], f[0]);

         subd_tris.push_back({f[0], v4, v6});
         subd_tris.push_back({f[1], v5, v4});
         subd_tris.push_back({f[2], v6, v5});
         subd_tris.push_back({v4, v5, v6});
      }

      std::swap(faces, subd_tris);
   };

   for (auto i = 0; i < subdivisions; ++i) subdivide();

   auto normals = positions;

   for (auto& pos : positions) pos *= radius;

   return {positions, normals, spherical_unwrap(positions), faces};
}

}
