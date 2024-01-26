#include "Calculations.h"

#include "Math/Intersections.h"

#include <glm/gtc/matrix_transform.hpp>

namespace LM
{

    glm::mat4 GetGrindingWheelMatrix(float _OffsetToolCenter, float _OffsetToolAxis, float _ToolAngle,
                                     float _RotatinOffset)
    {
        return glm::translate(glm::mat4(1.0f), { _OffsetToolAxis, _OffsetToolCenter, _RotatinOffset }) *
               glm::rotate(glm::mat4(1.0f), glm::radians(90.0f - _ToolAngle), glm::vec3(0.0f, -1.0f, 0.0f));
    }

    float MoveOverToolAxisRotationRadToOffset(float _RotationRad, float _ToolDiametr, float _ToolAngle)
    {
        return (_RotationRad * _ToolDiametr / 2.0f) * glm::tan(glm::radians(_ToolAngle));
    }

    float MoveOverToolAxisOffsetToRotationRad(float _Offset, float _ToolDiametr, float _ToolAngle)
    {
        return (_Offset / glm::tan(glm::radians(_ToolAngle))) / (_ToolDiametr / 2.0f);
    }

    MoveOverToolAxis CalcMoveOverToolAxis(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                                          const GrindingWheelProfileParams& _WheelProfileParams,
                                          const ToolParams& _ToolParams)
    {
        MoveOverToolAxis result;

        result.Max.Offset = glm::sin(glm::radians(90.0f - _ToolParams.Angle)) * _WheelParams.Width / 2.0f;
        result.Max.RotationRad =
            MoveOverToolAxisOffsetToRotationRad(result.Max.Offset, _ToolParams.Diametr, _ToolParams.Angle);

        glm::mat4 minRotationMatrix = GetGrindingWheelMatrix(
            _WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis, _ToolParams.Angle, 0.0f);

        glm::vec4 minRotationR1End = minRotationMatrix * _ShapeParams.R1End;
        glm::vec4 minRotationR2Start = minRotationMatrix * _ShapeParams.R2Start;

        glm::vec2 rightOnToolNoAngle =
            LineCircleIntersection(_ToolParams.Diametr / 2.0f, minRotationR1End, minRotationR2Start);

        float dX = rightOnToolNoAngle.x - _WheelProfileParams.OffsetToolAxis;

        result.Min.Offset = -glm::tan(glm::radians(90.0f - _ToolParams.Angle)) * dX;
        result.Min.RotationRad =
            MoveOverToolAxisOffsetToRotationRad(result.Min.Offset, _ToolParams.Diametr, _ToolParams.Angle);
            //(result.Min.Offset / glm::tan(glm::radians(_ToolParams.Angle))) / (_ToolParams.Diametr / 2.0f);

        return result;
    }

}    // namespace LM
