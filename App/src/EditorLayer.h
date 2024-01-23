#pragma once

#include "Engine/Buffers/FrameBuffer.h"
#include "Engine/Buffers/VertexArray.h"
#include "Engine/Layers/Layer.h"
#include "Engine/Shader/Shader.h"

namespace LM
{

    struct DrawableShape
    {
    public:
        Ref<VertexBuffer> m_Vertices;
        Ref<IndexBuffer> m_Indices;
        Ref<VertexArray> m_VertexArray;

        void Create(const std::vector<glm::vec4>& _Vertices, const std::vector<uint32_t>& _Indices);
        void Draw() const;
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

    struct CheckCorrectParams
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

    struct BestResult
    {
        float Width = 0.0f;
        float R1 = 0.0f;
        float R2 = 0.0f;
        float Angle = 0.0f;
        float OffsetToolCenter = 0.0f;
        float OffsetToolAxis = 0.0f;

        float Diametr = 0.0f;

        float FrontAngle;

    };

    struct CalculationResult
    {
        bool IsBad = false;

        float FrontAngle;
    };

    class EditorLayer : public Layer
    {
    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;

    protected:
        static glm::mat4 GetGrindingWheelMatrix(float _OffsetToolCenter, float _OffsetToolAxis, float _ToolAngle,
                                                float _RotatinOffset);

        static bool IsInsideTool(const glm::vec4 _Point, float _ToolDiametr);

        static bool IsWheelCorrect(const CheckCorrectParams& _CheckParams,
                                   const const GrindingWheelParams& _WheelParams, const glm::mat4& _Matrix,
                                   float _ToolDiametr);

        static CheckCorrectParams CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams);

        void Calculate();

        void SetAutoCameraZoom();

        void CreateGrindingWheelShape();
        void CreateToolShape();

        void CreateWheelShapeFromCalcParams(const GrindingWheelCalcParams& _CalcParams);

        void ImGuiDrawWheelCalcParams();

        void DrawTopMenu();
        void DrawAll();

    protected:
        DrawableShape m_WheelShape;
        DrawableShape m_ToolShape;

        Ref<Shader> m_Shader;
        Ref<FrameBuffer> m_FrameBuffer;

        float m_CameraAngleX = 0.0f;
        float m_CameraZoom = 0.0f;

        bool m_NeedDrawGrindingWheelStartShape = false;

        bool m_UseDepthTest = false;
        bool m_DrawPolygonsAsLines = false;

        GrindingWheelParams m_GrindingWheelParams;
        GrindingWheelProfileParams m_GrindingWheelProfileParams;
        CalcParams m_GrindingWheelCalcParams;

        float m_GrindingWheelCalcRatation = 0.0f;

        ToolParams m_ToolParams;

        int m_BadCalclulations = 0;
        std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<CalculationResult>>>>>>
            m_CalculationResults;

        bool m_HasBestResult = false;
        BestResult m_BestResult;
    };

}    // namespace LM
