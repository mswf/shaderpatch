#pragma once

#include <glm/glm.hpp>

namespace sp::editor {

struct Aa_bbox {
   glm::vec3 min;
   glm::vec3 max;
};

inline Aa_bbox transform(const Aa_bbox& bbox, const glm::mat4& matrix)
{
   Aa_bbox result;

   result.min = glm::vec4{bbox.min, 1.0f} * matrix;
   result.max = glm::vec4{bbox.max, 1.0f} * matrix;

   return result;
}

inline Aa_bbox bbox_combine(const Aa_bbox& left, const Aa_bbox& right) noexcept
{
   Aa_bbox bbox;

   bbox.min = glm::min(glm::min(left.min, left.max), glm::min(right.min, right.max));
   bbox.max = glm::max(glm::max(left.min, left.max), glm::max(right.min, right.max));

   return bbox;
}

template<typename Sequence>
inline Aa_bbox bbox_from_sequence(const Sequence& sequence)
{
   Aa_bbox bbox{};

   for (const auto& v : sequence) {
      bbox.min = glm::min(bbox.min, v);
      bbox.max = glm::max(bbox.max, v);
   }

   return bbox;
}

}
