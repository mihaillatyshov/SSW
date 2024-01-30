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
        float RotationAngle;
    };

    struct ToolParams
    {
        float Diametr;
        float Height;
        float Angle;
    };

    template <typename T>
    struct GrindingWheelCalcTemplate
    {
        T Diametr;
        T Width;
        T R1;
        T R2;
        T Angle;
        T OffsetToolCenter;
        T OffsetToolAxis;
        T RotationAngle;
    };

    typedef GrindingWheelCalcTemplate<float> GrindingWheelCalcParams;

    typedef GrindingWheelCalcTemplate<int> GrindingWheelCalcSteps;

    struct CalcParams
    {
        GrindingWheelCalcParams Min;
        GrindingWheelCalcParams Max;
        GrindingWheelCalcSteps Steps;
    };

    struct ParamsToFind
    {
        float FrontAngle = 0.0f;
        float StepAngle = 0.0f;
        float DiametrIn = 0.0f;
    };

    struct BestResult
    {
        float Width = 0.0f;
        float R1 = 0.0f;
        float R2 = 0.0f;
        float Angle = 0.0f;
        float OffsetToolCenter = 0.0f;
        float OffsetToolAxis = 0.0f;
        float RotationAngle = 0.0f;

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

    struct BestResultMeta
    {
        int Calculated = 0;
        int BadCalculations = 0;

        bool HasBestResult;
    };

    glm::mat4 GetGrindingWheelMatrix(float _OffsetToolCenter, float _OffsetToolAxis, float _ToolAngle,
                                     float _RotatinOffset);

    float MoveOverToolAxisRotationRadToOffset(float _RotationRad, float _ToolDiametr, float _ToolAngle);
    float MoveOverToolAxisOffsetToRotationRad(float _Offset, float _ToolDiametr, float _ToolAngle);

    MoveOverToolAxis CalcMoveOverToolAxis(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                                          const GrindingWheelProfileParams& _WheelProfileParams,
                                          const ToolParams& _ToolParams);

    bool IsInsideTool(const glm::vec4 _Point, float _ToolDiametr);

    bool IsWheelCorrect(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                        const glm::mat4& _Matrix, float _ToolDiametr);

    ShapeParams CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams);

    void CalculateBestResultSingle(const GrindingWheelParams& _WheelParams,
                                   const GrindingWheelProfileParams& _WheelProfileParams, const ToolParams& _ToolParams,
                                   const ParamsToFind& _ParamsToFind, ParamsToFind* _NearestParamsToFind,
                                   float* _LowestDelta, BestResult* _BestResult, BestResultMeta* _Meta);

}    // namespace LM
