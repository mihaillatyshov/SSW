#pragma once

#include <glm/glm.hpp>

namespace LM
{

    struct ShapeParams
    {
        glm::vec4 LeftCenterPoint;
        glm::vec4 RightCenterPoint;
        glm::vec4 R1Center;
        glm::vec4 R1Start;
        glm::vec4 R1End;
        glm::vec4 R2Center;
        glm::vec4 R2Start;
        glm::vec4 R2End;
    };

    struct GrindingWheelParams
    {
        float Diametr;
        float Width;
        float R1;
        float R2;
        float Angle;
    };

    struct GrindingWheelProfileParams
    {
        float OffsetToolCenter;
        float OffsetToolAxis;
    };

    struct ToolParams
    {
        float Diametr;
        float Height;
        float Angle;
    };

    struct GrindingWheelCalcParams
    {
        float Width;
        float R1;
        float R2;
        float Angle;
        float OffsetToolCenter;
        float OffsetToolAxis;
    };

    struct GrindingWheelCalcSteps
    {
        int Width;
        int R1;
        int R2;
        int Angle;
        int OffsetToolCenter;
        int OffsetToolAxis;
    };

    struct CalcParams
    {
        float Diametr;
        GrindingWheelCalcParams Min;
        GrindingWheelCalcParams Max;
        GrindingWheelCalcSteps Steps;
    };

    struct BestResult
    {
        float Width = 0.0f;
        float R1 = 0.0f;
        float R2 = 0.0f;
        float Angle = 0.0f;
        float OffsetToolCenter = 0.0f;
        float OffsetToolAxis = 0.0f;

        float Diametr = 0.0f;

        float FrontAngle = 0.0f;
        float StepAngle = 0.0f;
        float DiametrIn = 0.0f;
    };

    struct MoveOverToolAxisSingle
    {
        float Offset;
        float RotationRad;
    };

    struct MoveOverToolAxis
    {
        MoveOverToolAxisSingle Max;
        MoveOverToolAxisSingle Min;
    };

    glm::mat4 GetGrindingWheelMatrix(float _OffsetToolCenter, float _OffsetToolAxis, float _ToolAngle,
                                     float _RotatinOffset);

    float MoveOverToolAxisRotationRadToOffset(float _RotationRad, float _ToolDiametr, float _ToolAngle);
    float MoveOverToolAxisOffsetToRotationRad(float _Offset, float _ToolDiametr, float _ToolAngle);

    MoveOverToolAxis CalcMoveOverToolAxis(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                                          const GrindingWheelProfileParams& _WheelProfileParams,
                                          const ToolParams& _ToolParams);

}    // namespace LM
