#pragma once

#include <glm/glm.hpp>

namespace sp::editor {

struct Texture;

struct Material_params {
   glm::vec3 base_color{1.0f, 1.0f, 1.0f};
   float roughness = 1.0f;
   float metallicness = 0.0f;
   float ao_strength = 1.0f;
   float emissive_strength = 1.0f;
};

class Material {
public:
   Material_params params{};

   Texture& albedo_map();
   Texture& normal_map();
   Texture& metallic_roughness_map();
   Texture& ao_map();
   Texture& emissive_map();
};

}
