#include "Length.h"

namespace LM
{

    float Vec2Length(const glm::vec4& _Vec) { return glm::sqrt(_Vec.x * _Vec.x + _Vec.y * _Vec.y); }
    float Vec2Length(const glm::vec2& _Vec) { return glm::sqrt(_Vec.x * _Vec.x + _Vec.y * _Vec.y); }

}    // namespace LM
