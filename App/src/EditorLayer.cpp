#include "EditorLayer.h"

#include <algorithm>

#include "Engine/Core/Application.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace LM
{

    const size_t k_Sections = 36;
    const float PI = glm::pi<float>();
    constexpr float k_MaxFloat = std::numeric_limits<float>::max();
    constexpr float k_MinFloat = std::numeric_limits<float>::lowest();

    struct CurcleCurveProps
    {
        float AngleStart;
        float AngleEnd;
        float Radius;
        float CenterX;
        float CenterY;
    };

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

    float Vec2Length(const glm::vec4& _Vec) { return glm::sqrt(_Vec.x * _Vec.x + _Vec.y * _Vec.y); }
    float Vec2Length(const glm::vec2& _Vec) { return glm::sqrt(_Vec.x * _Vec.x + _Vec.y * _Vec.y); }

    float ValueByStep(float _Min, float _Max, int _Step, int _StepsCount)
    {
        return _Min + (_Max - _Min) * float(_Step) / float(_StepsCount);
    }

    std::vector<float> GenValueByStep(float _Min, float _Max, int _StepsCount)
    {
        std::vector<float> result;
        for (int i = 0; i <= _StepsCount; i++)
        {
            result.emplace_back(ValueByStep(_Min, _Max, i, _StepsCount));
        }
        return result;
    }

    std::vector<int> GenSteps(int _StepsCount)
    {
        std::vector<int> result;
        for (int i = 0; i <= _StepsCount; i++)
        {
            result.emplace_back(i);
        }
        return result;
    }

    float CalcAngle(glm::vec4 _Vec1, glm::vec4 _Vec2)
    {
        return glm::degrees(
            glm::acos((_Vec1.x * _Vec2.x + _Vec1.y * _Vec2.y) / (Vec2Length(_Vec1) * Vec2Length(_Vec2))));
    }

    void ImGuiHideCursorOnEdit()
    {
        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        if (ImGui::IsAnyItemActive() && ImGui::IsAnyMouseDown())
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void AlignForWidth(float width, float alignment = 0.5f)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        float avail = ImGui::GetContentRegionAvail().x;
        float off = (avail - width) * alignment;
        if (off > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }
    }

    void AddCircleCurve(CurcleCurveProps _Props, size_t _Sections, std::vector<glm::vec4>& _Vertices)
    {
        for (size_t i = 0; i <= _Sections; i++)
        {
            float angle = ValueByStep(_Props.AngleStart, _Props.AngleEnd, i, _Sections);

            float x = _Props.Radius * glm::cos(glm::radians(angle)) + _Props.CenterX;
            float y = _Props.Radius * glm::sin(glm::radians(angle)) + _Props.CenterY;

            _Vertices.emplace_back(x, y, 0.0f, 1.0f);
        }
    }

    void IndicesAddTriangle(std::vector<uint32_t>& _Indices, uint32_t _VertId0, uint32_t _VertId1, uint32_t _VertId2)
    {
        _Indices.emplace_back(_VertId0);
        _Indices.emplace_back(_VertId1);
        _Indices.emplace_back(_VertId2);
    }

    void DrawableShape::Create(const std::vector<glm::vec4>& _Vertices, const std::vector<uint32_t>& _Indices)
    {
        BufferLayout VerticesLayout({
            {ShaderDataType::Float4, "a_Position"},
        });

        m_Vertices = VertexBuffer::Create(_Vertices.data(), _Vertices.size() * VerticesLayout.GetStride());
        m_Vertices->SetLayout(VerticesLayout);

        m_Indices = IndexBuffer::Create(_Indices.data(), _Indices.size());

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(m_Vertices);
        m_VertexArray->SetIndexBuffer(m_Indices);
    }

    void DrawableShape::Draw() const
    {
        m_VertexArray->Bind();
        glDrawElements(GL_TRIANGLES, m_Indices->GetCount(), GL_UNSIGNED_INT, NULL);
    }

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

    glm::mat4 EditorLayer::GetGrindingWheelMatrix(float _OffsetToolCenter, float _OffsetToolAxis, float _ToolAngle,
                                                  float _RotatinOffset)
    {
        return glm::translate(glm::mat4(1.0f), { _OffsetToolAxis, _OffsetToolCenter, _RotatinOffset }) *
               glm::rotate(glm::mat4(1.0f), glm::radians(90.0f - _ToolAngle), glm::vec3(0.0f, -1.0f, 0.0f));
    }

    bool EditorLayer::IsInsideTool(const glm::vec4 _Point, float _ToolDiametr)
    {
        return Vec2Length(_Point) <= _ToolDiametr / 2.0f;
    }

    bool EditorLayer::IsWheelCorrect(const CheckCorrectParams& _CheckParams,
                                     const const GrindingWheelParams& _WheelParams, const glm::mat4& _Matrix,
                                     float _ToolDiametr)
    {
        bool checkRadiusSizeSum = ((_CheckParams.R1End.x - _CheckParams.R1Start.x) +
                                   (_CheckParams.R2End.x - _CheckParams.R2Start.x)) < _WheelParams.Width;

        bool checkDiametLeft = _CheckParams.LeftCenterPoint.y > _CheckParams.R1Start.y;
        bool checkDiametRight = _CheckParams.RightCenterPoint.y > _CheckParams.R2End.y;

        bool checkDiametLeftInTool = !IsInsideTool(_Matrix * _CheckParams.LeftCenterPoint, _ToolDiametr);
        bool checkDiametRightInTool = !IsInsideTool(_Matrix * _CheckParams.RightCenterPoint, _ToolDiametr);

        bool checkR1StartInTool = IsInsideTool(_Matrix * _CheckParams.R1Start, _ToolDiametr);
        bool checkR1EndInTool = IsInsideTool(_Matrix * _CheckParams.R1End, _ToolDiametr);

        bool checkR2StartNotInTool = !IsInsideTool(_Matrix * _CheckParams.R2Start, _ToolDiametr);
        bool checkR2EndNotInTool = !IsInsideTool(_Matrix * _CheckParams.R2End, _ToolDiametr);

        return checkRadiusSizeSum && (checkDiametLeft && checkDiametRight) &&
               (checkDiametLeftInTool && checkDiametRightInTool) && (checkR1StartInTool && checkR1EndInTool) &&
               (checkR2StartNotInTool && checkR2EndNotInTool);
    }

    CheckCorrectParams EditorLayer::CalculateGrindingWheelSizes(const GrindingWheelParams& _WheelParams)
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

        CheckCorrectParams result;

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

        float nearestFrontAngle = k_MaxFloat;
        float nearestStepAngle = k_MaxFloat;
        float nearestDiametrIn = k_MaxFloat;

        float lowestDelta = k_MaxFloat;

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
                                    (maxOffset / glm::tan(glm::radians(toolAngle))) / (m_ToolParams.Diametr / 2.0f);
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

                                glm::vec4 minRotationRightCenter =
                                    minRotationMatrix * calculatedParams.RightCenterPoint;
                                glm::vec4 minRotationR2End = minRotationMatrix * calculatedParams.R2End;

                                glm::vec2 rightOnTool =
                                    LineCircleIntersection(toolRadius, minRotationRightCenter, minRotationR2End);

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
                                               k_Sections, vertices);

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
                       k_Sections, vertices);
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
                       k_Sections, vertices);
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

        AddCircleCurve({ 0.0f, 360.0f, m_ToolParams.Diametr / 2.0f, 0.0f, 0.0f }, k_Sections * 10, vertices);
        for (size_t i = 2; i < vertices.size(); i++)
        {
            IndicesAddTriangle(indices, 0, i - 1, i);
        }

        m_ToolShape.Create(vertices, indices);
    }

    void EditorLayer::OnImGuiRender()
    {
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

        ImGuiHideCursorOnEdit();
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

    void EditorLayer::ImGuiDrawWheelCalcParams()
    {
        ImGui::Columns(2);
        ImGui::NextColumn();
        float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
        float secondColWidth = ImGui::GetContentRegionAvail().x;
        ImGui::NextColumn();

        ImGui::Text("Wheel Diametr");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth - itemSpacing);
        ImGui::DragFloat("##Wheel Diametr", &m_GrindingWheelCalcParams.Diametr, 0.1f, 1.0f, k_MaxFloat);
        ImGui::NextColumn();

        ImGui::Text("Wheel Width");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Width Min", &m_GrindingWheelCalcParams.Min.Width, 0.1f, 1.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Width Max", &m_GrindingWheelCalcParams.Max.Width, 0.1f, 1.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel Width Step", &m_GrindingWheelCalcParams.Steps.Width, 0.1f, 1, 100'000);
        ImGui::NextColumn();

        ImGui::Text("Wheel R1");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel R1 Min", &m_GrindingWheelCalcParams.Min.R1, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel R1 Max", &m_GrindingWheelCalcParams.Max.R1, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel R1 Step", &m_GrindingWheelCalcParams.Steps.R1, 0.1f, 1, 100'000);
        ImGui::NextColumn();

        ImGui::Text("Wheel R2");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel R2 Min", &m_GrindingWheelCalcParams.Min.R2, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel R2 Max", &m_GrindingWheelCalcParams.Max.R2, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel R2 Step", &m_GrindingWheelCalcParams.Steps.R2, 0.1f, 1, 100'000);
        ImGui::NextColumn();

        ImGui::Text("Wheel Angle");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Angle Min", &m_GrindingWheelCalcParams.Min.Angle, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Angle Max", &m_GrindingWheelCalcParams.Max.Angle, 0.1f, 0.0f, k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel Angle Step", &m_GrindingWheelCalcParams.Steps.Angle, 0.1f, 1, 100'000);
        ImGui::NextColumn();

        ImGui::Text("Wheel Center Offset");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Center Offset Min", &m_GrindingWheelCalcParams.Min.OffsetToolCenter, 0.1f, 0.0f,
                         k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Center Offset Max", &m_GrindingWheelCalcParams.Max.OffsetToolCenter, 0.1f, 0.0f,
                         k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel Center Offset Step", &m_GrindingWheelCalcParams.Steps.OffsetToolCenter, 0.1f, 1,
                       100'000);
        ImGui::NextColumn();

        ImGui::Text("Wheel Axis Offset");
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Axis Offset Min", &m_GrindingWheelCalcParams.Min.OffsetToolAxis, 0.1f, k_MinFloat,
                         k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragFloat("##Wheel Axis Offset Max", &m_GrindingWheelCalcParams.Max.OffsetToolAxis, 0.1f, k_MinFloat,
                         k_MaxFloat);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(secondColWidth / 3.0f - itemSpacing);
        ImGui::DragInt("##Wheel Axis Offset Step", &m_GrindingWheelCalcParams.Steps.OffsetToolAxis, 0.1f, 1, 100'000);
        ImGui::NextColumn();

        ImGui::Columns(1);
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

        float rotatinOffset = (glm::radians(m_GrindingWheelCalcRatation) * m_ToolParams.Diametr / 2.0f) *
                              glm::tan(glm::radians(m_ToolParams.Angle));
        m_Shader->SetUniform4f("u_Color", { 1.0f, 0.0f, 0.0f, 1.0f });
        m_Shader->SetUniformMat4(
            "u_ModelMatrix",
            glm::rotate(glm::mat4(1.0f), glm::radians(m_GrindingWheelCalcRatation), glm::vec3(0.0f, 0.0f, 1.0f)) *
                GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
                                       m_GrindingWheelProfileParams.OffsetToolAxis, m_ToolParams.Angle, rotatinOffset));
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
            bool WheelParamsChanged = false;
            bool ToolParamsChanged = false;

            ImGui::SeparatorText("Tool");

            if (ImGui::DragFloat("Tool Diametr", &m_ToolParams.Diametr, 0.1f, 1.0f, k_MaxFloat))
            {
                ToolParamsChanged = true;
            }
            if (ImGui::DragFloat("Tool Angle", &m_ToolParams.Angle, 0.01f, 15.0f, 90.0f))
            {
                ToolParamsChanged = true;
            }
            if (ImGui::DragFloat("Tool Height", &m_ToolParams.Height, 0.1f, 1.0f, k_MaxFloat))
            {
                ToolParamsChanged = true;
            }

            if (ToolParamsChanged)
            {
                CreateToolShape();
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

            if (ImGui::DragFloat("Wheel Diametr", &m_GrindingWheelParams.Diametr, 0.1f, 1.0f, k_MaxFloat))
            {
                WheelParamsChanged = true;
            }
            if (ImGui::DragFloat("Wheel Width", &m_GrindingWheelParams.Width, 0.1f, 1.0f, k_MaxFloat))
            {
                WheelParamsChanged = true;
            }
            if (ImGui::DragFloat("Wheel R1", &m_GrindingWheelParams.R1, 0.1f, 0.0f, k_MaxFloat))
            {
                WheelParamsChanged = true;
            }
            if (ImGui::DragFloat("Wheel R2", &m_GrindingWheelParams.R2, 0.1f, 0.0f, k_MaxFloat))
            {
                WheelParamsChanged = true;
            }
            if (ImGui::DragFloat("Wheel Angle", &m_GrindingWheelParams.Angle, 0.1f, 0.0f, k_MaxFloat))
            {
                WheelParamsChanged = true;
            }

            ImGui::DragFloat("Wheel Center Offset", &m_GrindingWheelProfileParams.OffsetToolCenter, 0.1f, 0.0f,
                             k_MaxFloat);
            ImGui::DragFloat("Wheel Axis Offset", &m_GrindingWheelProfileParams.OffsetToolAxis, 0.1f, k_MinFloat,
                             k_MaxFloat);

            bool isCorrect = IsWheelCorrect(CalculateGrindingWheelSizes(m_GrindingWheelParams), m_GrindingWheelParams,
                                            GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
                                                                   m_GrindingWheelProfileParams.OffsetToolAxis,
                                                                   m_ToolParams.Angle, 0.0f),
                                            m_ToolParams.Diametr);
            ImGui::Checkbox("Is Correct", &isCorrect);

            if (WheelParamsChanged)
            {
                CreateGrindingWheelShape();
            }
        }
        ImGui::End();

        if (ImGui::Begin("Rendering"))
        {
            ImGui::DragFloat("Camera Angle X", &m_CameraAngleX, 0.1f, -60.0f, 60.0f);

            ImGui::DragFloat("Camera Zoom", &m_CameraZoom, 0.01f, 1.0f, std::numeric_limits<float>::max());

            ImGui::Separator();

            ImGui::Checkbox("Draw Grinding Wheel Start Shape", &m_NeedDrawGrindingWheelStartShape);

            float maxOffset = glm::sin(glm::radians(90.0f - m_ToolParams.Angle)) * m_GrindingWheelParams.Width / 2.0f;
            float maxCalcRotation =
                glm::degrees((maxOffset / glm::tan(glm::radians(m_ToolParams.Angle))) / (m_ToolParams.Diametr / 2.0f));
            ImGui::DragFloat("Grinding Wheel Profile Rotation", &m_GrindingWheelCalcRatation, 0.001f, -maxCalcRotation,
                             maxCalcRotation);

            ImGui::Text("Real: maxOffset: %f", maxOffset);
            ImGui::Text("Test: maxOffset: %f", (glm::radians(maxCalcRotation) * m_ToolParams.Diametr / 2.0f) *
                                                   glm::tan(glm::radians(m_ToolParams.Angle)));
            ImGui::Text("Test: rotatinOffset: %f",
                        (glm::radians(m_GrindingWheelCalcRatation) * m_ToolParams.Diametr / 2.0f) *
                            glm::tan(glm::radians(m_ToolParams.Angle)));

            ImGui::Separator();
            ImGui::Checkbox("Use DepthTest", &m_UseDepthTest);
            ImGui::Checkbox("Draw Polygons as Lines", &m_DrawPolygonsAsLines);
        }
        ImGui::End();

        if (ImGui::Begin("Output Wheel"))
        {
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();

            float imgSize = glm::min(regionAvail.x, regionAvail.y);
            AlignForWidth(imgSize);
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
