#include "EditorLayer.h"

#include <algorithm>

#include "Calculations/Steps.h"
#include "Graphics/GraphicsUtils.h"
#include "Gui/CustomGui.h"
#include "Math/Angle.h"
#include "Math/Intersections.h"
#include "Math/Length.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace LM
{

    const size_t kSections = 36;
    const float PI = glm::pi<float>();
    constexpr float kMaxFloat = std::numeric_limits<float>::max();
    constexpr float kMinFloat = std::numeric_limits<float>::lowest();

    void EditorLayer::OnAttach()
    {
        m_GrindingWheelParams.Diametr = 240.0f;
        m_GrindingWheelParams.Width = 15.0f;
        m_GrindingWheelParams.R1 = 2.0f;
        m_GrindingWheelParams.R2 = 1.0f;
        m_GrindingWheelParams.Angle = 15.0f;

        m_GrindingWheelCalcParams.Diametr = 240.0f;

        m_GrindingWheelCalcParams.Min.Width = 15.0f;
        m_GrindingWheelCalcParams.Min.R1 = 2.0f;
        m_GrindingWheelCalcParams.Min.R2 = 1.0f;
        m_GrindingWheelCalcParams.Min.Angle = 15.0f;
        m_GrindingWheelCalcParams.Min.OffsetToolCenter = 100.0f;
        m_GrindingWheelCalcParams.Min.OffsetToolAxis = -20.0f;

        m_GrindingWheelCalcParams.Max.Width = 80.0f;
        m_GrindingWheelCalcParams.Max.R1 = 10.0f;
        m_GrindingWheelCalcParams.Max.R2 = 5.0f;
        m_GrindingWheelCalcParams.Max.Angle = 45.0f;
        m_GrindingWheelCalcParams.Max.OffsetToolCenter = 140.0f;
        m_GrindingWheelCalcParams.Max.OffsetToolAxis = 20.0f;

        m_GrindingWheelCalcParams.Steps.Width = 50;
        m_GrindingWheelCalcParams.Steps.R1 = 15;
        m_GrindingWheelCalcParams.Steps.R2 = 5;
        m_GrindingWheelCalcParams.Steps.Angle = 20;
        m_GrindingWheelCalcParams.Steps.OffsetToolCenter = 50;
        m_GrindingWheelCalcParams.Steps.OffsetToolAxis = 50;

        m_ToolParams.Diametr = 100.0f;
        m_ToolParams.Height = 600.0f;
        m_ToolParams.Angle = 60.0f;

        m_GrindingWheelProfileParams.OffsetToolCenter =
            m_ToolParams.Diametr / 2.0f + m_GrindingWheelParams.Diametr / 2.0f * 0.625f;
        m_GrindingWheelProfileParams.OffsetToolAxis = -(m_GrindingWheelParams.Width * 0.125f);

        SetAutoCameraZoom();

        m_Shader = Shader::Create(ShaderLayout(
            {
                {ShaderDataType::Float4, "a_Position"},
        },
            { { ShaderSource::Type::VERTEX, ShaderSource::FromFile("assets/shaders/test.vert") },
              { ShaderSource::Type::FRAGMENT, ShaderSource::FromFile("assets/shaders/test.frag") } }));

        CreateGrindingWheelShape();
        CreateToolShape();

        m_FrameBuffer = FrameBuffer::Create({ 2048, 2048, { FrameBufferColorMASK::NONE }, FrameBufferMASK::DEPTH });
    }

    void EditorLayer::OnDetach() { }

    bool EditorLayer::IsInsideTool(const glm::vec4 _Point, float _ToolDiametr)
    {
        return Vec2Length(_Point) <= _ToolDiametr / 2.0f;
    }

    bool EditorLayer::IsWheelCorrect(const ShapeParams& _ShapeParams, const GrindingWheelParams& _WheelParams,
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

    ShapeParams EditorLayer::CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams)
    {
        float r1CenterX = -_WheelParams.Width / 2.0f + _WheelParams.R1;
        float r1DX = glm::sin(glm::radians(_WheelParams.Angle)) * _WheelParams.R1;
        float r1DEndX = _WheelParams.R1 + r1DX;
        float r1PointEndX = -_WheelParams.Width / 2.0f + r1DEndX;
        float r1DY = glm::cos(glm::radians(_WheelParams.Angle)) * _WheelParams.R1;
        float r1DEndY = glm::tan(glm::radians(_WheelParams.Angle)) * r1DEndX;
        float r1PointEndY = -_WheelParams.Diametr / 2.0f + r1DEndY;
        float r1CenterY = -_WheelParams.Diametr / 2.0f + r1DEndY + r1DY;

        float r2CenterX = _WheelParams.Width / 2.0f - _WheelParams.R2;
        float r2DX = glm::sin(glm::radians(_WheelParams.Angle)) * _WheelParams.R2;
        float r2DStartX = _WheelParams.R2 - r2DX;
        float r2DY = glm::cos(glm::radians(_WheelParams.Angle)) * _WheelParams.R2;

        float r1r2DX = _WheelParams.Width - r1DEndX - r2DStartX;
        float r1r2DY = glm::tan(glm::radians(_WheelParams.Angle)) * r1r2DX;

        float r2PointStartX = r1PointEndX + r1r2DX;
        float r2PointStartY = r1PointEndY + r1r2DY;
        float r2CenterY = r2PointStartY + r2DY;

        ShapeParams result;

        result.LeftCenterPoint = { -_WheelParams.Width / 2.0f, 0.0f, 0.0f, 1.0f };
        result.RightCenterPoint = { _WheelParams.Width / 2.0f, 0.0f, 0.0f, 1.0f };

        result.R1Center = { r1CenterX, r1CenterY, 0.0f, 1.0f };
        result.R1Start = { result.LeftCenterPoint.x, r1CenterY, 0.0f, 1.0f };
        result.R1End = { r1PointEndX, r1PointEndY, 0.0f, 1.0f };

        result.R2Center = { r2CenterX, r2CenterY, 0.0f, 1.0f };
        result.R2Start = { r2PointStartX, r2PointStartY, 0.0f, 1.0f };
        result.R2End = { result.RightCenterPoint.x, r2CenterY, 0.0f, 1.0f };

        return result;
    }

    void EditorLayer::Calculate()
    {
        auto startTime = std::chrono::system_clock::now();

        std::vector<int> widthStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.Width);
        std::vector<int> r1StepArr = GenSteps(m_GrindingWheelCalcParams.Steps.R1);
        std::vector<int> r2StepArr = GenSteps(m_GrindingWheelCalcParams.Steps.R2);
        std::vector<int> angleStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.Angle);
        std::vector<int> offsetToolCenterStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.OffsetToolCenter);
        std::vector<int> offsetToolAxisStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.OffsetToolAxis);

        int maxCalculations = (m_GrindingWheelCalcParams.Steps.Width + 1) * (m_GrindingWheelCalcParams.Steps.R1 + 1) *
                              (m_GrindingWheelCalcParams.Steps.R2 + 1) * (m_GrindingWheelCalcParams.Steps.Angle + 1) *
                              (m_GrindingWheelCalcParams.Steps.OffsetToolCenter + 1) *
                              (m_GrindingWheelCalcParams.Steps.OffsetToolAxis + 1);

        // m_CalculationResults.clear();

        float toolAngle = m_ToolParams.Angle;
        float toolDiametr = m_ToolParams.Diametr;
        float toolRadius = m_ToolParams.Diametr / 2.0f;

        float wheelDiametr = m_GrindingWheelCalcParams.Diametr;
        int badCalculations = 0;
        int calculated = 0;

        float nearestFrontAngle = kMaxFloat;
        float nearestStepAngle = kMaxFloat;
        float nearestDiametrIn = kMaxFloat;

        float lowestDelta = kMaxFloat;

        float frontAngleToFind = 5.0f;
        float stepAngleToFind = 50.0f;
        float diametrInToFind = 75.0f;

#define SH_FOR(var, Var) std::for_each(var##StepArr.begin(), var##StepArr.end(), [=, &badCalculations, &calculated, &nearestFrontAngle, &nearestStepAngle, &lowestDelta, &nearestDiametrIn](int var##Step) { \
            float var = ValueByStep(m_GrindingWheelCalcParams.Min.Var, m_GrindingWheelCalcParams.Max.Var, var##Step, m_GrindingWheelCalcParams.Steps.Var);
#define SH_FOR_END                                                                                                     \
    })

        SH_FOR(width, Width)
        {
            LOGD();
            LOGW("Calculating: ", width);
            SH_FOR(r1, R1)
            {
                SH_FOR(r2, R2)
                {
                    SH_FOR(angle, Angle)
                    {
                        SH_FOR(offsetToolCenter, OffsetToolCenter)
                        {
                            SH_FOR(offsetToolAxis, OffsetToolAxis)
                            {
                                calculated++;
                                GrindingWheelParams params = { wheelDiametr, width, r1, r2, angle };
                                auto calculatedParams = CalculateGrindingWheelSizes(params);

                                glm::mat4 wheelMatrix0 =
                                    GetGrindingWheelMatrix(offsetToolCenter, offsetToolAxis, toolAngle, 0.0f);

                                if (!IsWheelCorrect(calculatedParams, params, wheelMatrix0, toolDiametr))
                                {
                                    badCalculations++;
                                    // m_CalculationResults[widthStep][r1Step][r2Step][angleStep][offsetToolCenterStep]
                                    //                     [offsetToolAxisStep] = { true, 0.0f, 0.0f };
                                    return;
                                }

                                float maxOffset = glm::sin(glm::radians(90.0f - toolAngle)) * width / 2.0f;
                                float maxOffsetRotationRad =
                                    (maxOffset / glm::tan(glm::radians(toolAngle))) / (toolRadius);
                                glm::mat4 maxRotationMatrix =
                                    glm::rotate(glm::mat4(1.0f), -maxOffsetRotationRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                    GetGrindingWheelMatrix(offsetToolCenter, offsetToolAxis, toolAngle, -maxOffset);

                                glm::vec4 maxRotationLeftCenter = maxRotationMatrix * calculatedParams.LeftCenterPoint;
                                glm::vec4 maxRotationR1Start = maxRotationMatrix * calculatedParams.R1Start;

                                glm::vec2 leftOnTool =
                                    LineCircleIntersection(toolRadius, maxRotationLeftCenter, maxRotationR1Start);

                                float frontAngle = CalcAngle(maxRotationR1Start - maxRotationLeftCenter,
                                                             glm::vec4(-leftOnTool, 0.0f, 1.0f));

                                glm::mat4 minRotationMatrix =
                                    glm::rotate(glm::mat4(1.0f), maxOffsetRotationRad, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                    GetGrindingWheelMatrix(offsetToolCenter, offsetToolAxis, toolAngle, maxOffset);

                                glm::vec4 minRotationR1End = minRotationMatrix * calculatedParams.R1End;
                                glm::vec4 minRotationR2Start = minRotationMatrix * calculatedParams.R2Start;

                                glm::vec2 rightOnTool =
                                    LineCircleIntersection(toolRadius, minRotationR1End, minRotationR2Start);

                                float stepAngle =
                                    CalcAngle(glm::vec4(rightOnTool, 0.0f, 1.0f), glm::vec4(leftOnTool, 0.0f, 1.0f));

                                if (isnan(stepAngle) || isnan(frontAngle))
                                {
                                    badCalculations++;
                                    return;
                                }

                                std::vector<glm::vec4> vertices;
                                AddCircleCurve({ 180.0f, 270.0f + angle, r1, calculatedParams.R1Center.x,
                                                 calculatedParams.R1Center.y },
                                               kSections, vertices);

                                float diametrIn = 2.0f * LineToPointDistance(wheelMatrix0 * calculatedParams.R1End,
                                                                             wheelMatrix0 * calculatedParams.R2Start,
                                                                             glm::vec2(0.0f));
                                for (const glm::vec4& vert : vertices)
                                {
                                    diametrIn = glm::min(diametrIn, 2.0f * Vec2Length(wheelMatrix0 * vert));
                                }

                                if (isnan(diametrIn))
                                {
                                    badCalculations++;
                                    return;
                                }

                                float deltaFrontAngle = glm::abs(frontAngle - frontAngleToFind);
                                if (deltaFrontAngle < glm::abs(nearestFrontAngle - frontAngleToFind))
                                {
                                    nearestFrontAngle = frontAngle;
                                }
                                float deltaStepAngle = glm::abs(stepAngle - stepAngleToFind);
                                if (deltaStepAngle < glm::abs(nearestStepAngle - stepAngleToFind))
                                {
                                    nearestStepAngle = stepAngle;
                                }
                                float deltaDiametrIn = glm::abs(diametrIn - diametrInToFind);
                                if (deltaDiametrIn < glm::abs(nearestDiametrIn - diametrInToFind))
                                {
                                    nearestDiametrIn = diametrIn;
                                }

                                float delta = deltaFrontAngle + deltaStepAngle + deltaDiametrIn;
                                if (delta < lowestDelta)
                                {
                                    lowestDelta = delta;
                                    m_BestResult.Width = width;
                                    m_BestResult.R1 = r1;
                                    m_BestResult.R2 = r2;
                                    m_BestResult.Angle = angle;
                                    m_BestResult.OffsetToolCenter = offsetToolCenter;
                                    m_BestResult.OffsetToolAxis = offsetToolAxis;

                                    m_BestResult.FrontAngle = frontAngle;
                                    m_BestResult.StepAngle = stepAngle;
                                    m_BestResult.DiametrIn = diametrIn;

                                    m_BestResult.Diametr = wheelDiametr;

                                    m_HasBestResult = true;
                                }
                            }
                            SH_FOR_END;
                        }
                        SH_FOR_END;
                    }
                    SH_FOR_END;
                }
                SH_FOR_END;
            }
            SH_FOR_END;
        }
        SH_FOR_END;

#define SH_VALUE_BY_STEP(var, Var)                                                                                     \
    ValueByStep(m_GrindingWheelCalcParams.Min.Var, m_GrindingWheelCalcParams.Max.Var, var##Step,                       \
                m_GrindingWheelCalcParams.Steps.Var)

        auto endTime = std::chrono::system_clock::now();

        LOGW("Nearest FrontAngle: ", nearestFrontAngle);
        LOGW("Nearest StepAngle: ", nearestStepAngle);
        LOGW("Nearest Diametr In: ", nearestDiametrIn);
        LOGW("Max Calculations: ", maxCalculations);
        LOGW("Bad Calculations: ", float(badCalculations) / float(maxCalculations) * 100.0f, "% (", badCalculations,
             ")");
        LOGW("Calculation Time: ",
             std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0, "s");
    }

    void EditorLayer::SetAutoCameraZoom()
    {
        m_CameraZoom =
            glm::max(m_GrindingWheelParams.Diametr + m_ToolParams.Diametr, m_GrindingWheelParams.Width) / 2.0f;
    }

    void EditorLayer::CreateGrindingWheelShape()
    {
        std::vector<glm::vec4> vertices = {
            {0.0f, 0.0f, 0.0f, 1.0f}
        };
        std::vector<uint32_t> indices;

        auto wheelParams = CalculateGrindingWheelSizes(m_GrindingWheelParams);

        vertices.emplace_back(wheelParams.LeftCenterPoint);
        uint32_t leftCenterVertId = vertices.size() - 1;

        float r1CenterX = -m_GrindingWheelParams.Width / 2.0f + m_GrindingWheelParams.R1;
        float r1DX = glm::sin(glm::radians(m_GrindingWheelParams.Angle)) * m_GrindingWheelParams.R1;
        float r1DEndX = m_GrindingWheelParams.R1 + r1DX;
        float r1PointEndX = -m_GrindingWheelParams.Width / 2.0f + r1DEndX;
        float r1DY = glm::cos(glm::radians(m_GrindingWheelParams.Angle)) * m_GrindingWheelParams.R1;
        float r1DEndY = glm::tan(glm::radians(m_GrindingWheelParams.Angle)) * r1DEndX;
        float r1PointEndY = -m_GrindingWheelParams.Diametr / 2.0f + r1DEndY;
        float r1CenterY = -m_GrindingWheelParams.Diametr / 2.0f + r1DEndY + r1DY;

        uint32_t r1StartVertId = vertices.size();
        AddCircleCurve({ 180.0f, 270.0f + m_GrindingWheelParams.Angle, m_GrindingWheelParams.R1, r1CenterX, r1CenterY },
                       kSections, vertices);
        uint32_t r1EndVertId = vertices.size() - 1;

        for (size_t i = r1StartVertId + 1; i < vertices.size(); i++)
        {
            IndicesAddTriangle(indices, 0, i - 1, i);
        }

        float r2CenterX = m_GrindingWheelParams.Width / 2.0f - m_GrindingWheelParams.R2;
        float r2DX = glm::sin(glm::radians(m_GrindingWheelParams.Angle)) * m_GrindingWheelParams.R2;
        float r2DStartX = m_GrindingWheelParams.R2 - r2DX;
        float r2DY = glm::cos(glm::radians(m_GrindingWheelParams.Angle)) * m_GrindingWheelParams.R2;

        float r1r2DX = m_GrindingWheelParams.Width - r1DEndX - r2DStartX;
        float r1r2DY = glm::tan(glm::radians(m_GrindingWheelParams.Angle)) * r1r2DX;
        float r2PointStartX = r1PointEndX + r1r2DX;
        float r2PointStartY = r1PointEndY + r1r2DY;
        float r2CenterY = r2PointStartY + r2DY;

        uint32_t r2StartVertId = vertices.size();
        AddCircleCurve({ 270.0f + m_GrindingWheelParams.Angle, 360.0f, m_GrindingWheelParams.R2, r2CenterX, r2CenterY },
                       kSections, vertices);
        uint32_t r2EndVertId = vertices.size() - 1;

        vertices.emplace_back(wheelParams.RightCenterPoint);
        uint32_t rightCenterVertId = vertices.size() - 1;

        for (size_t i = r2StartVertId + 1; i < vertices.size(); i++)
        {
            IndicesAddTriangle(indices, 0, i - 1, i);
        }

        IndicesAddTriangle(indices, 0, r1EndVertId, r2StartVertId);
        IndicesAddTriangle(indices, 0, leftCenterVertId, r1StartVertId);
        IndicesAddTriangle(indices, 0, r2EndVertId, rightCenterVertId);

        m_WheelShape.Create(vertices, indices);
    }

    void EditorLayer::CreateToolShape()
    {
        std::vector<glm::vec4> vertices = {
            {0.0f, 0.0f, 0.0f, 1.0f}
        };
        std::vector<uint32_t> indices;

        AddCircleCurve({ 0.0f, 360.0f, m_ToolParams.Diametr / 2.0f, 0.0f, 0.0f }, kSections * 10, vertices);
        for (size_t i = 2; i < vertices.size(); i++)
        {
            IndicesAddTriangle(indices, 0, i - 1, i);
        }

        m_ToolShape.Create(vertices, indices);
    }

    void EditorLayer::OnImGuiRender()
    {
        Gui::NewFrame();

        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                            ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
        {
            ImGui::PopStyleVar(2);
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        DrawTopMenu();

        ImGui::End();

        DrawAll();

        ImGui::ShowDemoWindow();

        Gui::EndFrame();
    }

    void EditorLayer::CreateWheelShapeFromCalcParams(const GrindingWheelCalcParams& _CalcParams)
    {
        m_GrindingWheelParams.Width = _CalcParams.Width;
        m_GrindingWheelParams.R1 = _CalcParams.R1;
        m_GrindingWheelParams.R2 = _CalcParams.R2;
        m_GrindingWheelParams.Angle = _CalcParams.Angle;

        m_GrindingWheelProfileParams.OffsetToolCenter = _CalcParams.OffsetToolCenter;
        m_GrindingWheelProfileParams.OffsetToolAxis = _CalcParams.OffsetToolAxis;

        CreateGrindingWheelShape();
    }

#define SH_DRAG_FLOAT_STEP(label, var, speed, min, max)                                                                \
    Gui::DragFloatStep(label, &m_GrindingWheelCalcParams.Min.var, &m_GrindingWheelCalcParams.Max.var,                  \
                       &m_GrindingWheelCalcParams.Steps.var, speed, min, max)

    void EditorLayer::ImGuiDrawWheelCalcParams()
    {
        if (Gui::BeginPropsTable("Inputs"))
        {
            Gui::DragFloat("Wheel Diametr", &m_GrindingWheelCalcParams.Diametr, 0.1f, 1.0f, kMaxFloat);

            SH_DRAG_FLOAT_STEP("Wheel Width", Width, 0.1f, 1.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel R1", R1, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel R2", R2, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Angle", Angle, 0.1f, 0.0f, 80.0f);
            SH_DRAG_FLOAT_STEP("Wheel Center Offset", OffsetToolCenter, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Axis Offset", OffsetToolAxis, 0.1f, kMinFloat, kMaxFloat);

            Gui::EndPropsTable();
        }
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (m_DrawPolygonsAsLines)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        if (m_UseDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        static glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_FrameBuffer->Bind();
        m_FrameBuffer->Clear(color);

        m_Shader->Enable();

        glm::mat4 projection =
            glm::ortho(-m_CameraZoom, m_CameraZoom, -m_CameraZoom, m_CameraZoom, -10000.0f, 10000.0f);

        glm::vec3 cameraEyeStart = { 0.0f, 0.0f, 1.0f };
        glm::vec3 cameraUpStart = { 0.0f, 1.0f, 0.0f };
        glm::vec3 cameraEye = glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f)) *
                              glm::vec4(cameraEyeStart, 1.0f);
        glm::vec3 cameraUp = glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f)) *
                             glm::vec4(cameraUpStart, 1.0f);
        glm::mat4 view = glm::lookAt(cameraEye, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

        m_Shader->SetUniformMat4("u_ProjectionMatrix", projection);
        m_Shader->SetUniformMat4("u_ViewMatrix", view);

        m_Shader->SetUniform4f("u_Color", { 0.0f, 0.0f, 1.0f, 1.0f });
        m_Shader->SetUniformMat4("u_ModelMatrix", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.0001f)));
        m_ToolShape.Draw();

        if (m_NeedDrawGrindingWheelStartShape)
        {
            m_Shader->SetUniform4f("u_Color", { 1.0f, 1.0f, 1.0f, 1.0f });
            m_Shader->SetUniformMat4("u_ModelMatrix", glm::mat4(1.0f));
            m_WheelShape.Draw();
        }

        float rotationOffset = MoveOverToolAxisRotationRadToOffset(glm::radians(m_GrindingWheelCalcRatation),
                                                                   m_ToolParams.Diametr, m_ToolParams.Angle);
        m_Shader->SetUniform4f("u_Color", { 1.0f, 0.0f, 0.0f, 1.0f });
        m_Shader->SetUniformMat4(
            "u_ModelMatrix",
            glm::rotate(glm::mat4(1.0f), glm::radians(m_GrindingWheelCalcRatation), glm::vec3(0.0f, 0.0f, 1.0f)) *
                GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
                                       m_GrindingWheelProfileParams.OffsetToolAxis, m_ToolParams.Angle,
                                       rotationOffset));
        m_WheelShape.Draw();

        // glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, m_Commands.data(), m_Commands.size(), 0);

        m_Shader->Disable();
        m_FrameBuffer->Unbind();
    }

    void EditorLayer::DrawAll()
    {
        if (ImGui::Begin("Calculation"))
        {
            if (ImGui::Button("Start Calculation"))
            {
                Calculate();
            }
            if (m_HasBestResult)
            {
                ImGui::SeparatorText("Best Result");
                ImGui::Text("Front Angle: %f", m_BestResult.FrontAngle);
                ImGui::Text("Step Angle: %f", m_BestResult.StepAngle);
                ImGui::Text("Diametr In: %f", m_BestResult.DiametrIn);
                ImGui::Text("Width: %f", m_BestResult.Width);
                ImGui::Text("R1: %f", m_BestResult.R1);
                ImGui::Text("R2: %f", m_BestResult.R2);
                ImGui::Text("Angle: %f", m_BestResult.Angle);
                ImGui::Text("OffsetToolCenter: %f", m_BestResult.OffsetToolCenter);
                ImGui::Text("OffsetToolAxis: %f", m_BestResult.OffsetToolAxis);

                if (ImGui::Button("Draw Best Result"))
                {
                    m_GrindingWheelParams.Diametr = m_BestResult.Diametr;
                    m_GrindingWheelParams.Width = m_BestResult.Width;
                    m_GrindingWheelParams.R1 = m_BestResult.R1;
                    m_GrindingWheelParams.R2 = m_BestResult.R2;
                    m_GrindingWheelParams.Angle = m_BestResult.Angle;

                    m_GrindingWheelProfileParams.OffsetToolCenter = m_BestResult.OffsetToolCenter;
                    m_GrindingWheelProfileParams.OffsetToolAxis = m_BestResult.OffsetToolAxis;

                    CreateGrindingWheelShape();
                }
            }
        }
        ImGui::End();

        if (ImGui::Begin("Inputs"))
        {
            bool toolParamsChanged = false;
            bool wheelParamsChanged = false;

            ImGui::SeparatorText("Tool");

            if (Gui::BeginPropsTable("Inputs"))
            {
                if (Gui::DragFloat("Tool Diametr", &m_ToolParams.Diametr, 0.1f, 1.0f, kMaxFloat))
                {
                    toolParamsChanged = true;
                }
                if (Gui::DragFloat("Tool Angle", &m_ToolParams.Angle, 0.01f, 15.0f, 90.0f))
                {
                    toolParamsChanged = true;
                }
                if (Gui::DragFloat("Tool Height", &m_ToolParams.Height, 0.1f, 1.0f, kMaxFloat))
                {
                    toolParamsChanged = true;
                }

                Gui::EndPropsTable();
            }

            ImGui::SeparatorText("Wheel");
            ImGuiDrawWheelCalcParams();

            ImGui::SeparatorText("Rendered Wheel");

            if (ImGui::Button("SetAsMinCalc"))
            {
                CreateWheelShapeFromCalcParams(m_GrindingWheelCalcParams.Min);
            }
            ImGui::SameLine();
            if (ImGui::Button("SetAsMaxCalc"))
            {
                CreateWheelShapeFromCalcParams(m_GrindingWheelCalcParams.Max);
            }

            if (Gui::BeginPropsTable("Inputs"))
            {
                if (Gui::DragFloat("Wheel Diametr", &m_GrindingWheelParams.Diametr, 0.1f, 1.0f, kMaxFloat))
                {
                    wheelParamsChanged = true;
                }
                if (Gui::DragFloat("Wheel Width", &m_GrindingWheelParams.Width, 0.1f, 1.0f, kMaxFloat))
                {
                    wheelParamsChanged = true;
                }
                if (Gui::DragFloat("Wheel R1", &m_GrindingWheelParams.R1, 0.1f, 0.0f, kMaxFloat))
                {
                    wheelParamsChanged = true;
                }
                if (Gui::DragFloat("Wheel R2", &m_GrindingWheelParams.R2, 0.1f, 0.0f, kMaxFloat))
                {
                    wheelParamsChanged = true;
                }
                if (Gui::DragFloat("Wheel Angle", &m_GrindingWheelParams.Angle, 0.1f, 0.0f, kMaxFloat))
                {
                    wheelParamsChanged = true;
                }

                Gui::DragFloat("Wheel Center Offset", &m_GrindingWheelProfileParams.OffsetToolCenter, 0.1f, 0.0f,
                               kMaxFloat);
                Gui::DragFloat("Wheel Axis Offset", &m_GrindingWheelProfileParams.OffsetToolAxis, 0.1f, kMinFloat,
                               kMaxFloat);

                Gui::EndPropsTable();
            }

            bool isCorrect = IsWheelCorrect(CalculateGrindingWheelSizes(m_GrindingWheelParams), m_GrindingWheelParams,
                                            GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
                                                                   m_GrindingWheelProfileParams.OffsetToolAxis,
                                                                   m_ToolParams.Angle, 0.0f),
                                            m_ToolParams.Diametr);
            ImGui::Text("Is Correct:");
            ImGui::SameLine();
            ImGui::Checkbox("##Is Correct", &isCorrect);

            if (toolParamsChanged)
            {
                CreateToolShape();
            }
            if (wheelParamsChanged)
            {
                CreateGrindingWheelShape();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Rendering"))
        {
            auto shapeParams = CalculateGrindingWheelSizes(m_GrindingWheelParams);

            MoveOverToolAxis moveOverToolAxis =
                CalcMoveOverToolAxis(shapeParams, m_GrindingWheelParams, m_GrindingWheelProfileParams, m_ToolParams);

            // float maxOffset = glm::sin(glm::radians(90.0f - m_ToolParams.Angle)) * m_GrindingWheelParams.Width
            // / 2.0f; float maxCalcRotation =
            //     glm::degrees((maxOffset / glm::tan(glm::radians(m_ToolParams.Angle))) / (m_ToolParams.Diametr
            //     / 2.0f));

            // glm::mat4 minRotationMatrix =
            //     GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
            //                            m_GrindingWheelProfileParams.OffsetToolAxis, m_ToolParams.Angle, 0.0f);

            // glm::vec4 minRotationR1End = minRotationMatrix * calculatedParams.R1End;
            // glm::vec4 minRotationR2Start = minRotationMatrix * calculatedParams.R2Start;

            // glm::vec2 rightOnToolNoAngle =
            //     LineCircleIntersection(m_ToolParams.Diametr / 2.0f, minRotationR1End, minRotationR2Start);

            // LOGD();

            // float dX = rightOnToolNoAngle.x - m_GrindingWheelProfileParams.OffsetToolAxis;
            // float minOffset = -glm::tan(glm::radians(90.0f - m_ToolParams.Angle)) * dX;
            // float minCalcRotation =
            //     glm::degrees((minOffset / glm::tan(glm::radians(m_ToolParams.Angle))) / (m_ToolParams.Diametr
            //     / 2.0f));

            // LOGW("maxRotationR1End: ", minRotationR1End.x, "  ", minRotationR1End.y);
            // LOGW("maxRotationR2Start: ", minRotationR2Start.x, "  ", minRotationR2Start.y);
            // LOGW("dx: ", dX);
            // LOGW("rightOnTool: ", rightOnToolNoAngle.x, "  ", rightOnToolNoAngle.y);
            // LOGW("maxOffset: ", maxOffset);
            // LOGW("maxCalcRotation: ", maxCalcRotation);

            if (Gui::BeginPropsTable("Rendering"))
            {
                Gui::DragFloat("Camera Angle X", &m_CameraAngleX, 0.1f, -60.0f, 60.0f);

                Gui::DragFloat("Camera Zoom", &m_CameraZoom, 0.01f, 1.0f, std::numeric_limits<float>::max());

                Gui::DragFloat("Grinding Wheel Profile Rotation", &m_GrindingWheelCalcRatation, 0.001f,
                               glm::degrees(moveOverToolAxis.Min.RotationRad),
                               glm::degrees(moveOverToolAxis.Max.RotationRad));

                Gui::EndPropsTable();
            }

            ImGui::Checkbox("Use DepthTest", &m_UseDepthTest);
            ImGui::Checkbox("Draw Polygons as Lines", &m_DrawPolygonsAsLines);
            ImGui::Checkbox("Draw Grinding Wheel Start Shape", &m_NeedDrawGrindingWheelStartShape);

            ImGui::Separator();

            ImGui::Text("Real: minOffset: %f", moveOverToolAxis.Min.Offset);
            ImGui::Text("Test: minOffset: %f",
                        MoveOverToolAxisRotationRadToOffset(moveOverToolAxis.Min.RotationRad, m_ToolParams.Diametr,
                                                            m_ToolParams.Angle));

            ImGui::Separator();

            ImGui::Text("Real: maxOffset: %f", moveOverToolAxis.Max.Offset);
            ImGui::Text("Test: maxOffset: %f",
                        MoveOverToolAxisRotationRadToOffset(moveOverToolAxis.Max.RotationRad, m_ToolParams.Diametr,
                                                            m_ToolParams.Angle));

            ImGui::Separator();

            ImGui::Text("Test: rotatinOffset: %f",
                        MoveOverToolAxisRotationRadToOffset(glm::radians(m_GrindingWheelCalcRatation), m_ToolParams.Diametr,
                                                            m_ToolParams.Angle));
        }
        ImGui::End();

        if (ImGui::Begin("Output Wheel"))
        {
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();

            float imgSize = glm::min(regionAvail.x, regionAvail.y);
            Gui::AlignForWidth(imgSize);
            ImGui::Image(m_FrameBuffer->GetTextureId(), { imgSize, imgSize }, { 0, 1 }, { 1, 0 });
        }
        ImGui::End();
    }

    void EditorLayer::DrawTopMenu()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                {
                }

                if (ImGui::MenuItem("New Project", "Ctrl+N"))
                {
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                }

                if (ImGui::MenuItem("Save Project As...", "Ctrl+Shift+S"))
                {
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Close", "Ctrl+F4"))
                {
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                {
                    Application::Get().Close();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

}    // namespace LM
