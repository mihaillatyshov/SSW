#include "Intersections.h"

namespace LM
{

    float SGN(float _Val) { return _Val < 0.0f ? -1.0f : 1.0f; }

    glm::vec2 LineCircleIntersection(float _ToolRadius, const glm::vec2& _Vec1, const glm::vec2& _Vec2)
    {
        float dx = _Vec2.x - _Vec1.x;
        float dy = _Vec2.y - _Vec1.y;
        float dr = glm::sqrt(dx * dx + dy * dy);

        float d = _Vec1.x * _Vec2.y - _Vec2.x * _Vec1.y;

        // float discriminant = _ToolRadius * _ToolRadius * dr * dr - d * d;

        float x1 = (d * dy + SGN(dy) * dx * glm::sqrt(_ToolRadius * _ToolRadius * dr * dr - d * d)) / (dr * dr);
        float x2 = (d * dy - SGN(dy) * dx * glm::sqrt(_ToolRadius * _ToolRadius * dr * dr - d * d)) / (dr * dr);
        float y1 = (-d * dx + glm::abs(dy) * glm::sqrt(_ToolRadius * _ToolRadius * dr * dr - d * d)) / (dr * dr);
        float y2 = (-d * dx - glm::abs(dy) * glm::sqrt(_ToolRadius * _ToolRadius * dr * dr - d * d)) / (dr * dr);

        return { x1, y1 };
    }

    float LineToPointDistance(const glm::vec2& _Vec1, const glm::vec2& _Vec2, const glm::vec2& _Point)
    {
        float t = ((_Point.x - _Vec1.x) * (_Vec2.x - _Vec1.x) + (_Point.y - _Vec1.y) * (_Vec2.y - _Vec1.y)) /
                  (glm::pow(_Vec2.x - _Vec1.x, 2.0f) + glm::pow(_Vec2.y - _Vec1.y, 2.0f));

        t = glm::clamp(t, 0.0f, 1.0f);

        return glm::sqrt(glm::pow(_Vec1.x + t * (_Vec2.x - _Vec1.x) - _Point.x, 2.0f) +
                         glm::pow(_Vec1.y + t * (_Vec2.y - _Vec1.y) - _Point.y, 2.0f));
    }

}    // namespace LM
