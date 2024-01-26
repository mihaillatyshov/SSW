#pragma once

#include <glm/glm.hpp>

namespace LM
{

    glm::vec2 LineCircleIntersection(float _ToolRadius, const glm::vec2& _Vec1, const glm::vec2& _Vec2);

    float LineToPointDistance(const glm::vec2& _Vec1, const glm::vec2& _Vec2, const glm::vec2& _Point);

}    // namespace LM
