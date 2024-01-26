#pragma once

#include "Engine/Buffers/FrameBuffer.h"
#include "Engine/Layers/Layer.h"
#include "Engine/Shader/Shader.h"

#include "Calculations/Calculations.h"
#include "Graphics/SimpleRenderable2D.h"

namespace LM
{

    class EditorLayer : public Layer
    {
    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;

    protected:
        static bool IsInsideTool(const glm::vec4 _Point, float _ToolDiametr);

        static bool IsWheelCorrect(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
                                   const glm::mat4& _Matrix, float _ToolDiametr);

        static ShapeParams CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams);

        void Calculate();

        void SetAutoCameraZoom();

        void CreateGrindingWheelShape();
        void CreateToolShape();

        void CreateWheelShapeFromCalcParams(const GrindingWheelCalcParams& _CalcParams);

        void ImGuiDrawWheelCalcParams();

        void DrawTopMenu();
        void DrawAll();

    protected:
        SimpleRenderable2D m_WheelShape;
        SimpleRenderable2D m_ToolShape;

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

        bool m_HasBestResult = false;
        BestResult m_BestResult;
    };

}    // namespace LM
