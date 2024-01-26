#include "Angle.h"

namespace LM
{

    float CalcAngle(glm::vec4 _Vec1, glm::vec4 _Vec2)
    {
        return glm::degrees(
            glm::acos((_Vec1.x * _Vec2.x + _Vec1.y * _Vec2.y) / (Vec2Length(_Vec1) * Vec2Length(_Vec2))));
    }

}    // namespace LM
