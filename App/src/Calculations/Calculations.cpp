#include "Calculations.h"

#include "Engine/Utils/ConsoleLog.h"

#include "Graphics/GraphicsUtils.h"
#include "Math/Angle.h"
#include "Math/Intersections.h"
#include "Math/Length.h"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace LM
{

    constexpr size_t kSections = 36;

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
        MoveOverToolAxis result = {};

        result.Max.Offset = 0.0f;
        result.Max.RotationRad = 0.0f;

        glm::mat4 minRotationMatrix =
            GetGrindingWheelMatrix(_WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis,
                                   _WheelProfileParams.RotationAngle, 0.0f);

        glm::vec4 minRotationR1End = minRotationMatrix * _ShapeParams.R1End;
        glm::vec4 minRotationR2Start = minRotationMatrix * _ShapeParams.R2Start;

        glm::vec2 rightOnToolNoAngle =
            LineCircleIntersection(_ToolParams.Diametr / 2.0f, minRotationR1End, minRotationR2Start);

        float dX = rightOnToolNoAngle.x - _WheelProfileParams.OffsetToolAxis;

        result.Min.Offset = -glm::tan(glm::radians(90.0f - _WheelProfileParams.RotationAngle)) * dX;
        result.Min.RotationRad =
            MoveOverToolAxisOffsetToRotationRad(result.Min.Offset, _ToolParams.Diametr, _ToolParams.Angle);
        //(result.Min.Offset / glm::tan(glm::radians(_ToolParams.Angle))) / (_ToolParams.Diametr / 2.0f);

        return result;
    }

    bool IsInsideTool(const glm::vec4 _Point, float _ToolDiametr) { return Vec2Length(_Point) <= _ToolDiametr / 2.0f; }

    bool IsWheelCorrect(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                        const glm::mat4& _Matrix, float _ToolDiametr)
    {
        bool checkRadiusSizeSum = ((_ShapeParams.R1End.x - _ShapeParams.R1Start.x) +
                                   (_ShapeParams.R2End.x - _ShapeParams.R2Start.x)) < _WheelParams.Width;

        bool checkDiametLeft = _ShapeParams.LeftCenterPoint.y > _ShapeParams.R1Start.y;
        bool checkDiametRight = _ShapeParams.RightCenterPoint.y > _ShapeParams.R2End.y;

        bool checkDiametLeftInTool = !IsInsideTool(_Matrix * _ShapeParams.LeftCenterPoint, _ToolDiametr);
        bool checkDiametRightInTool = !IsInsideTool(_Matrix * _ShapeParams.RightCenterPoint, _ToolDiametr);

        bool checkR1StartInTool = IsInsideTool(_Matrix * _ShapeParams.R1Start, _ToolDiametr);
        bool checkR1EndInTool = IsInsideTool(_Matrix * _ShapeParams.R1End, _ToolDiametr);

        bool checkR2StartNotInTool = !IsInsideTool(_Matrix * _ShapeParams.R2Start, _ToolDiametr);
        bool checkR2EndNotInTool = !IsInsideTool(_Matrix * _ShapeParams.R2End, _ToolDiametr);

        return checkRadiusSizeSum && (checkDiametLeft && checkDiametRight) &&
               (checkDiametLeftInTool && checkDiametRightInTool) && (checkR1StartInTool && checkR1EndInTool) &&
               (checkR2StartNotInTool && checkR2EndNotInTool);
    }

    ShapeParams CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams)
    {
        float r1CenterX = _WheelParams.R1;
        float r1DX = glm::sin(glm::radians(_WheelParams.Angle)) * _WheelParams.R1;
        float r1DEndX = _WheelParams.R1 + r1DX;
        float r1PointEndX = r1DEndX;
        float r1DY = glm::cos(glm::radians(_WheelParams.Angle)) * _WheelParams.R1;
        float r1DEndY = glm::tan(glm::radians(_WheelParams.Angle)) * r1DEndX;
        float r1PointEndY = r1DEndY;
        float r1CenterY = r1DEndY + r1DY;

        float r2CenterX = _WheelParams.Width - _WheelParams.R2;
        float r2DX = glm::sin(glm::radians(_WheelParams.Angle)) * _WheelParams.R2;
        float r2DStartX = _WheelParams.R2 - r2DX;
        float r2DY = glm::cos(glm::radians(_WheelParams.Angle)) * _WheelParams.R2;

        float r1r2DX = _WheelParams.Width - r1DEndX - r2DStartX;
        float r1r2DY = glm::tan(glm::radians(_WheelParams.Angle)) * r1r2DX;

        float r2PointStartX = r1PointEndX + r1r2DX;
        float r2PointStartY = r1PointEndY + r1r2DY;
        float r2CenterY = r2PointStartY + r2DY;

        ShapeParams result = {};

        result.LeftCenterPoint = { 0.0f, _WheelParams.Diametr / 2.0f, 0.0f, 1.0f };
        result.RightCenterPoint = { _WheelParams.Width, _WheelParams.Diametr / 2.0f, 0.0f, 1.0f };

        result.R1Center = { r1CenterX, r1CenterY, 0.0f, 1.0f };
        result.R1Start = { result.LeftCenterPoint.x, r1CenterY, 0.0f, 1.0f };
        result.R1End = { r1PointEndX, r1PointEndY, 0.0f, 1.0f };

        result.R2Center = { r2CenterX, r2CenterY, 0.0f, 1.0f };
        result.R2Start = { r2PointStartX, r2PointStartY, 0.0f, 1.0f };
        result.R2End = { result.RightCenterPoint.x, r2CenterY, 0.0f, 1.0f };

        return result;
    }

    void CalculateBestResultSingle(const GrindingWheelParams& _WheelParams,
                                   const GrindingWheelProfileParams& _WheelProfileParams, const ToolParams& _ToolParams,
                                   const ParamsToFind& _ParamsToFind, ParamsToFind* _NearestParamsToFind,
                                   float* _LowestDelta, BestResult* _BestResult, BestResultMeta* _Meta)
    {
        float toolRadius = _ToolParams.Diametr / 2.0f;

        _Meta->Calculated++;
        // GrindingWheelParams params = { diametr, width, r1, r2, angle };
        auto shapeParams = CalculateGrindingWheelSizes(_WheelParams);

        glm::mat4 wheelMatrix0 =
            GetGrindingWheelMatrix(_WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis,
                                   _WheelProfileParams.RotationAngle, 0.0f);

        // if (!IsWheelCorrect(shapeParams, _WheelParams, wheelMatrix0, _ToolParams.Diametr))
        //{
        //     _Meta->BadCalculations++;
        //     return;
        // }
        // LOGW("WHEEL CORRECT!!!");

        MoveOverToolAxis moveOverToolAxis =
            CalcMoveOverToolAxis(shapeParams, _WheelParams,
                                 { _WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis,
                                   _WheelProfileParams.RotationAngle },
                                 _ToolParams);

        // TODO: maybe need to remove maxRotationMatrix
        glm::mat4 maxRotationMatrix =
            glm::rotate(glm::mat4(1.0f), moveOverToolAxis.Max.RotationRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
            GetGrindingWheelMatrix(_WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis,
                                   _WheelProfileParams.RotationAngle, moveOverToolAxis.Max.Offset);

        glm::vec4 maxRotationLeftCenter = maxRotationMatrix * shapeParams.LeftCenterPoint;
        glm::vec4 maxRotationR1Start = maxRotationMatrix * shapeParams.R1Start;

        glm::vec2 leftOnTool = LineCircleIntersection(toolRadius, maxRotationLeftCenter, maxRotationR1Start);

        float frontAngle = CalcAngle(glm::vec4(-leftOnTool, 0.0f, 1.0f), maxRotationR1Start - maxRotationLeftCenter);
        if (isnan(frontAngle))
        {
            _Meta->BadCalculations++;
            return;
        }
        LOGW("WHEEL CORRECT!!!");

        // TODO: fix next time lower code
        glm::mat4 minRotationMatrix =
            glm::rotate(glm::mat4(1.0f), moveOverToolAxis.Min.RotationRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
            GetGrindingWheelMatrix(_WheelProfileParams.OffsetToolCenter, _WheelProfileParams.OffsetToolAxis,
                                   _WheelProfileParams.RotationAngle, moveOverToolAxis.Min.Offset);

        glm::vec4 minRotationR1End = minRotationMatrix * shapeParams.R1End;
        glm::vec4 minRotationR2Start = minRotationMatrix * shapeParams.R2Start;

        glm::vec2 rightOnTool = LineCircleIntersection(toolRadius, minRotationR1End, minRotationR2Start);

        float stepAngle = CalcAngle(glm::vec4(rightOnTool, 0.0f, 1.0f), glm::vec4(leftOnTool, 0.0f, 1.0f));
        if (isnan(stepAngle))
        {
            _Meta->BadCalculations++;
            return;
        }

        // std::vector<glm::vec4> vertices;
        // AddCircleCurve(
        //     { 180.0f, 270.0f + _WheelParams.Angle, _WheelParams.R1, shapeParams.R1Center.x, shapeParams.R1Center.y },
        //     kSections, vertices);

        float diametrIn = 2.0f * LineToPointDistance(wheelMatrix0 * shapeParams.R1End,
                                                     wheelMatrix0 * shapeParams.R2Start, glm::vec2(0.0f));
        // for (const glm::vec4& vert : vertices)
        //{
        //     diametrIn = glm::min(diametrIn, 2.0f * Vec2Length(wheelMatrix0 * vert));
        // }

        if (isnan(diametrIn))
        {
            _Meta->BadCalculations++;
            return;
        }

        // float deltaFrontAngle = glm::abs(frontAngle - _ParamsToFind.FrontAngle);
        // if (deltaFrontAngle < glm::abs(_NearestParamsToFind->FrontAngle - _ParamsToFind.FrontAngle))
        //{
        //     _NearestParamsToFind->FrontAngle = frontAngle;
        // }
        float deltaFrontAngle = 0.0f;
        _NearestParamsToFind->FrontAngle = frontAngle;
        float deltaStepAngle = glm::abs(stepAngle - _ParamsToFind.StepAngle);
        if (deltaStepAngle < glm::abs(_NearestParamsToFind->StepAngle - _ParamsToFind.StepAngle))
        {
            _NearestParamsToFind->StepAngle = stepAngle;
        }
        float deltaDiametrIn = glm::abs(diametrIn - _ParamsToFind.DiametrIn);
        if (deltaDiametrIn < glm::abs(_NearestParamsToFind->DiametrIn - _ParamsToFind.DiametrIn))
        {
            _NearestParamsToFind->DiametrIn = diametrIn;
        }

        float delta = deltaFrontAngle + deltaStepAngle + deltaDiametrIn;
        if (delta < (*_LowestDelta))
        {
            *_LowestDelta = delta;
            _BestResult->Diametr = _WheelParams.Diametr;
            _BestResult->Width = _WheelParams.Width;
            _BestResult->R1 = _WheelParams.R1;
            _BestResult->R2 = _WheelParams.R2;
            _BestResult->Angle = _WheelParams.Angle;
            _BestResult->OffsetToolCenter = _WheelProfileParams.OffsetToolCenter;
            _BestResult->OffsetToolAxis = _WheelProfileParams.OffsetToolAxis;
            _BestResult->RotationAngle = _WheelProfileParams.RotationAngle;

            _BestResult->FrontAngle = frontAngle;
            _BestResult->StepAngle = stepAngle;
            _BestResult->DiametrIn = diametrIn;

            _Meta->HasBestResult = true;
        }
    }

}    // namespace LM
