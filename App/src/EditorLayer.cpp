#include "EditorLayer.h"

#include <algorithm>
#include <execution>

#include "Engine/ImGui/Plots/implot.h"

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

    int MetricFormatter(double value, char* buff, int size, void* data)
    {
        const char* unit = (const char*)data;
        static double v[] = { 1000000000, 1000000, 1000, 1, 0.001, 0.000001, 0.000000001 };
        static const char* p[] = { "G", "M", "k", "", "m", "u", "n" };
        if (value == 0)
        {
            return snprintf(buff, size, "0 %s", unit);
        }
        for (int i = 0; i < 7; ++i)
        {
            if (fabs(value) >= v[i])
            {
                return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
            }
        }
        return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
    }

    template <typename T>
    int BinarySearch(const T* arr, int l, int r, T x)
    {
        if (r >= l)
        {
            int mid = l + (r - l) / 2;
            if (arr[mid] == x)
            {
                return mid;
            }
            if (arr[mid] > x)
            {
                return BinarySearch(arr, l, mid - 1, x);
            }
            return BinarySearch(arr, mid + 1, r, x);
        }
        return -1;
    }

    int InsideItemId(const std::vector<double>& XArr, double _MousePosX, double _Delta)
    {
        for (int i = 0; i < XArr.size(); i++)
        {
            if (glm::abs(XArr[i] - _MousePosX) < _Delta)
            {
                return i;
            }
        }
        return -1;
    }

    void DrawTooltip(const std::vector<double>& xs, const std::vector<double>& frontAngle,
                     const std::vector<double>& stepAngle, const std::vector<double>& diametrIn, float width_percent)
    {
        // get ImGui window DrawList
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();
        // calc real value width

        // custom tool
        if (ImPlot::IsPlotHovered())
        {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            LOGE("mouse", mouse.x, "  ", mouse.y);
            // mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
            float tool_l = ImPlot::PlotToPixels(mouse.x - width_percent, mouse.y).x;
            float tool_r = ImPlot::PlotToPixels(mouse.x + width_percent, mouse.y).x;
            float tool_t = ImPlot::GetPlotPos().y;
            float tool_b = tool_t + ImPlot::GetPlotSize().y;
            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();
            // find mouse location index
            int idx = InsideItemId(xs, mouse.x, width_percent);
            // render tool tip (won't be affected by plot clip rect)
            if (idx != -1)
            {
                ImGui::BeginTooltip();
                ImGui::Text("Prop:  %.4f ", xs[idx]);
                ImGui::Text("Front Angle: %.4f", frontAngle[idx]);
                ImGui::Text("Step Angle:  %.4f", stepAngle[idx]);
                ImGui::Text("Diametr In:  %.4f", diametrIn[idx]);
                ImGui::EndTooltip();
            }
        }
    }

    void EditorLayer::OnAttach()
    {
        m_GrindingWheelParams.Diametr = 240.0f;
        m_GrindingWheelParams.Width = 15.0f;
        m_GrindingWheelParams.R1 = 2.0f;
        m_GrindingWheelParams.R2 = 1.0f;
        m_GrindingWheelParams.Angle = 15.0f;

        m_GrindingWheelCalcParams.Min.Diametr = 200.0f;
        m_GrindingWheelCalcParams.Min.Width = 15.0f;
        m_GrindingWheelCalcParams.Min.R1 = 2.0f;
        m_GrindingWheelCalcParams.Min.R2 = 1.0f;
        m_GrindingWheelCalcParams.Min.Angle = 15.0f;
        m_GrindingWheelCalcParams.Min.OffsetToolCenter = 100.0f;
        m_GrindingWheelCalcParams.Min.OffsetToolAxis = -20.0f;
        m_GrindingWheelCalcParams.Min.RotationAngle = 55.0f;

        m_GrindingWheelCalcParams.Max.Diametr = 240.0f;
        m_GrindingWheelCalcParams.Max.Width = 85.0f;
        m_GrindingWheelCalcParams.Max.R1 = 10.0f;
        m_GrindingWheelCalcParams.Max.R2 = 5.0f;
        m_GrindingWheelCalcParams.Max.Angle = 45.0f;
        m_GrindingWheelCalcParams.Max.OffsetToolCenter = 140.0f;
        m_GrindingWheelCalcParams.Max.OffsetToolAxis = 20.0f;
        m_GrindingWheelCalcParams.Max.RotationAngle = 65.0f;

        m_GrindingWheelCalcParams.Steps.Diametr = 4;
        m_GrindingWheelCalcParams.Steps.Width = 70;
        m_GrindingWheelCalcParams.Steps.R1 = 16;
        m_GrindingWheelCalcParams.Steps.R2 = 4;
        m_GrindingWheelCalcParams.Steps.Angle = 15;
        m_GrindingWheelCalcParams.Steps.OffsetToolCenter = 40;
        m_GrindingWheelCalcParams.Steps.OffsetToolAxis = 40;
        m_GrindingWheelCalcParams.Steps.RotationAngle = 10;

        //m_GrindingWheelCalcParams.Max.Diametr = 200.0f;
        //m_GrindingWheelCalcParams.Max.Width = 54.0f;
        //m_GrindingWheelCalcParams.Max.R1 = 10.0f;
        //m_GrindingWheelCalcParams.Max.R2 = 5.0f;
        //m_GrindingWheelCalcParams.Max.Angle = 45.0f;
        //m_GrindingWheelCalcParams.Max.OffsetToolCenter = 134.0f;
        //m_GrindingWheelCalcParams.Max.OffsetToolAxis = 19.0f;
        //m_GrindingWheelCalcParams.Max.RotationAngle = 65.0f;

        //m_GrindingWheelCalcParams.Steps.Diametr = 1;
        //m_GrindingWheelCalcParams.Steps.Width = 1;
        //m_GrindingWheelCalcParams.Steps.R1 = 1;
        //m_GrindingWheelCalcParams.Steps.R2 = 1;
        //m_GrindingWheelCalcParams.Steps.Angle = 1;
        //m_GrindingWheelCalcParams.Steps.OffsetToolCenter = 1;
        //m_GrindingWheelCalcParams.Steps.OffsetToolAxis = 1;
        //m_GrindingWheelCalcParams.Steps.RotationAngle = 2;

        m_ToolParams.Diametr = 100.0f;
        m_ToolParams.Height = 600.0f;
        m_ToolParams.Angle = 60.0f;

        m_GrindingWheelProfileParams.OffsetToolCenter =
            m_ToolParams.Diametr / 2.0f + m_GrindingWheelParams.Diametr / 2.0f * 0.625f;
        m_GrindingWheelProfileParams.OffsetToolAxis = -(m_GrindingWheelParams.Width * 0.125f);
        m_GrindingWheelProfileParams.RotationAngle = 65.0f;

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

    void EditorLayer::Calculate()
    {
        auto startTime = std::chrono::system_clock::now();

        std::vector<int> diametrStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.Diametr);
        std::vector<int> widthStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.Width);
        std::vector<int> r1StepArr = GenSteps(m_GrindingWheelCalcParams.Steps.R1);
        std::vector<int> r2StepArr = GenSteps(m_GrindingWheelCalcParams.Steps.R2);
        std::vector<int> angleStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.Angle);
        std::vector<int> offsetToolCenterStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.OffsetToolCenter);
        std::vector<int> offsetToolAxisStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.OffsetToolAxis);
        std::vector<int> rotationAngleStepArr = GenSteps(m_GrindingWheelCalcParams.Steps.RotationAngle);

        int maxCalculations =
            (m_GrindingWheelCalcParams.Steps.Diametr + 1) * (m_GrindingWheelCalcParams.Steps.Width + 1) *
            (m_GrindingWheelCalcParams.Steps.R1 + 1) * (m_GrindingWheelCalcParams.Steps.R2 + 1) *
            (m_GrindingWheelCalcParams.Steps.Angle + 1) * (m_GrindingWheelCalcParams.Steps.OffsetToolCenter + 1) *
            (m_GrindingWheelCalcParams.Steps.OffsetToolAxis + 1) * (m_GrindingWheelCalcParams.Steps.RotationAngle + 1);

        // m_CalculationResults.clear();

        ParamsToFind paramsToFind;
        paramsToFind.FrontAngle = 5.0f;
        paramsToFind.StepAngle = 50.0f;
        paramsToFind.DiametrIn = 75.0f;

        size_t theadsCount = m_GrindingWheelCalcParams.Steps.Width + 1;

        std::vector<float> lowestDeltaArr(theadsCount, kMaxFloat);

        std::vector<ParamsToFind> nearestParamsToFindArr(theadsCount, { kMaxFloat, kMaxFloat, kMaxFloat });

        std::vector<BestResult> bestResultArr(theadsCount, BestResult());
        std::vector<BestResultMeta> metaArr(theadsCount, BestResultMeta());

#define SH_FOR(var, Var) std::for_each(var##StepArr.begin(), var##StepArr.end(), [=, &lowestDelta, &nearestParamsToFind, &bestResult, &meta](int var##Step) { \
            float var = ValueByStep(m_GrindingWheelCalcParams.Min.Var, m_GrindingWheelCalcParams.Max.Var, var##Step, m_GrindingWheelCalcParams.Steps.Var);
#define SH_FOR_END                                                                                                     \
    })

        LOGD();
        std::for_each(
            std::execution::par_unseq, widthStepArr.begin(), widthStepArr.end(),
            [=, &lowestDeltaArr, &nearestParamsToFindArr, &bestResultArr, &metaArr](int widthStep) {
                float width = ValueByStep(m_GrindingWheelCalcParams.Min.Width, m_GrindingWheelCalcParams.Max.Width,
                                          widthStep, m_GrindingWheelCalcParams.Steps.Width);

                float& lowestDelta = lowestDeltaArr[widthStep];
                ParamsToFind& nearestParamsToFind = nearestParamsToFindArr[widthStep];
                BestResult& bestResult = bestResultArr[widthStep];
                BestResultMeta& meta = metaArr[widthStep];

                LOGW("Calculating: ", width);
                SH_FOR(diametr, Diametr)
                SH_FOR(r1, R1)
                SH_FOR(r2, R2)
                SH_FOR(angle, Angle)
                SH_FOR(offsetToolCenter, OffsetToolCenter)
                SH_FOR(offsetToolAxis, OffsetToolAxis)
                SH_FOR(rotationAngle, RotationAngle)

                GrindingWheelParams params = { diametr, width, r1, r2, angle };
                GrindingWheelProfileParams profileParams = { offsetToolCenter, offsetToolAxis, rotationAngle };

                CalculateBestResultSingle(params, profileParams, m_ToolParams, paramsToFind, &nearestParamsToFind,
                                          &lowestDelta, &bestResult, &meta);

                SH_FOR_END;
                SH_FOR_END;
                SH_FOR_END;
                SH_FOR_END;
                SH_FOR_END;
                SH_FOR_END;
                SH_FOR_END;
            });

        LOGD();

        float lowestDelta = lowestDeltaArr[0];
        m_BestResult = bestResultArr[0];
        m_HasBestResult = metaArr[0].HasBestResult;

        for (int i = 1; i <= m_GrindingWheelCalcParams.Steps.Width; i++)
        {
            if (lowestDeltaArr[i] < lowestDelta)
            {
                lowestDelta = lowestDeltaArr[i];
                m_BestResult = bestResultArr[i];
                m_HasBestResult = metaArr[i].HasBestResult;
            }
        }

#define SH_VALUE_BY_STEP(var, Var)                                                                                     \
    ValueByStep(m_GrindingWheelCalcParams.Min.Var, m_GrindingWheelCalcParams.Max.Var, var##Step,                       \
                m_GrindingWheelCalcParams.Steps.Var)

        auto endTime = std::chrono::system_clock::now();

        /*LOGW("Nearest FrontAngle: ", nearestParamsToFind.FrontAngle);
        LOGW("Nearest StepAngle: ", nearestParamsToFind.StepAngle);
        LOGW("Nearest Diametr In: ", nearestParamsToFind.DiametrIn);
        LOGW("Max Calculations: ", maxCalculations);
        LOGW("Lowest Delta: ", lowestDelta);
        LOGW("Bad Calculations: ", float(meta.BadCalculations) / float(maxCalculations) * 100.0f, "% (",
             meta.BadCalculations, ")");*/
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
        m_GrindingWheelProfileParams.RotationAngle = _CalcParams.RotationAngle;

        CreateGrindingWheelShape();
    }

#define SH_DRAG_FLOAT_STEP(label, var, speed, min, max)                                                                \
    Gui::DragFloatStep(label, &m_GrindingWheelCalcParams.Min.var, &m_GrindingWheelCalcParams.Max.var,                  \
                       &m_GrindingWheelCalcParams.Steps.var, speed, min, max)

    void EditorLayer::ImGuiDrawWheelCalcParams()
    {
        if (Gui::BeginPropsTable("Inputs"))
        {
            SH_DRAG_FLOAT_STEP("Wheel Diametr", Diametr, 0.1f, 1.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Width", Width, 0.1f, 1.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel R1", R1, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel R2", R2, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Bottom Angle", Angle, 0.1f, 0.0f, 80.0f);
            SH_DRAG_FLOAT_STEP("Wheel Center Offset", OffsetToolCenter, 0.1f, 0.0f, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Axis Offset", OffsetToolAxis, 0.1f, kMinFloat, kMaxFloat);
            SH_DRAG_FLOAT_STEP("Wheel Rotation Angle", RotationAngle, 0.1f, 15.0f, 90.0f);

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
                                       m_GrindingWheelProfileParams.OffsetToolAxis,
                                       m_GrindingWheelProfileParams.RotationAngle, rotationOffset));
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
                ImGui::Separator();
                ImGui::Text("Width: %f", m_BestResult.Width);
                ImGui::Text("Diametr: %f", m_BestResult.Diametr);
                ImGui::Text("R1: %f", m_BestResult.R1);
                ImGui::Text("R2: %f", m_BestResult.R2);
                ImGui::Text("Angle: %f", m_BestResult.Angle);
                ImGui::Text("Offset Tool Center: %f", m_BestResult.OffsetToolCenter);
                ImGui::Text("Offset Tool Axis: %f", m_BestResult.OffsetToolAxis);
                ImGui::Text("Rotation Angle: %f", m_BestResult.RotationAngle);

                if (ImGui::Button("Draw Best Result"))
                {
                    m_GrindingWheelParams.Diametr = m_BestResult.Diametr;
                    m_GrindingWheelParams.Width = m_BestResult.Width;
                    m_GrindingWheelParams.R1 = m_BestResult.R1;
                    m_GrindingWheelParams.R2 = m_BestResult.R2;
                    m_GrindingWheelParams.Angle = m_BestResult.Angle;

                    m_GrindingWheelProfileParams.OffsetToolCenter = m_BestResult.OffsetToolCenter;
                    m_GrindingWheelProfileParams.OffsetToolAxis = m_BestResult.OffsetToolAxis;
                    m_GrindingWheelProfileParams.RotationAngle = m_BestResult.RotationAngle;

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
                Gui::DragFloat("Wheel Rotation Angle", &m_GrindingWheelProfileParams.RotationAngle, 0.1f, 15.0f,
                               kMaxFloat);

                Gui::EndPropsTable();
            }

            bool isCorrect = IsWheelCorrect(CalculateGrindingWheelSizes(m_GrindingWheelParams), m_GrindingWheelParams,
                                            GetGrindingWheelMatrix(m_GrindingWheelProfileParams.OffsetToolCenter,
                                                                   m_GrindingWheelProfileParams.OffsetToolAxis,
                                                                   m_GrindingWheelProfileParams.RotationAngle, 0.0f),
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
            //                            m_GrindingWheelProfileParams.OffsetToolAxis,
            //                            m_GrindingWheelProfileParams.RotationAngle, 0.0f);

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
                        MoveOverToolAxisRotationRadToOffset(glm::radians(m_GrindingWheelCalcRatation),
                                                            m_ToolParams.Diametr, m_ToolParams.Angle));
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

        DrawPlots();

        ImPlot::ShowDemoWindow();
    }

    void EditorLayer::DrawPlots()
    {
        if (ImGui::Begin("Tool Angle Plot"))
        {
            if (m_HasBestResult)
            {
                if (ImPlot::BeginPlot("Tool Angle Plot", ImVec2(-1, -1)))
                {
                    std::vector<double> toolAngleArr;
                    std::vector<double> frontAngleArr;
                    std::vector<double> stepAngleArr;
                    std::vector<double> diametrInArr;


                    for (int i = -5; i <= 5; i++)
                    {
                        float lowestDelta = kMaxFloat;
                        ParamsToFind nearestParamsToFind = { kMaxFloat, kMaxFloat, kMaxFloat };
                        BestResult bestResult;
                        BestResultMeta meta;

                        double toolAngle = double(m_ToolParams.Angle) + double(i);
                        toolAngleArr.emplace_back(toolAngle);

                        GrindingWheelParams params = { m_BestResult.Diametr, m_BestResult.Width, m_BestResult.R1,
                                                       m_BestResult.R2, m_BestResult.Angle };
                        GrindingWheelProfileParams profileParams = { m_BestResult.OffsetToolCenter,
                                                                     m_BestResult.OffsetToolAxis,
                                                                     m_BestResult.RotationAngle };

                        ToolParams toolParams = m_ToolParams;
                        toolParams.Angle = toolAngle;

                        CalculateBestResultSingle(params, profileParams, toolParams, { 0.0f, 0.0f, 0.0f },
                                                  &nearestParamsToFind, &lowestDelta, &bestResult, &meta);

                        frontAngleArr.emplace_back(bestResult.FrontAngle);
                        stepAngleArr.emplace_back(bestResult.StepAngle);
                        diametrInArr.emplace_back(bestResult.DiametrIn);
                    }

                    ImPlot::SetupAxes("ToolAngle", "");
                    ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);

                    ImPlot::SetupAxisFormat(ImAxis_X1, "%g deg");
                    ImPlot::SetupAxisFormat(ImAxis_Y1, "%g mm");
                    ImPlot::SetupAxisFormat(ImAxis_Y2, "%g deg");

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Diametr In", toolAngleArr.data(), diametrInArr.data(), toolAngleArr.size());

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Front Angle", toolAngleArr.data(), frontAngleArr.data(), toolAngleArr.size());
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Step Angle", toolAngleArr.data(), stepAngleArr.data(), toolAngleArr.size());
                    DrawTooltip(toolAngleArr, frontAngleArr, stepAngleArr, diametrInArr, 0.25f);
                    ImPlot::EndPlot();
                }
            }
        }
        ImGui::End();

        if (ImGui::Begin("Wheel Diametr Plot"))
        {
            if (m_HasBestResult)
            {
                if (ImPlot::BeginPlot("Wheel Diametr Plot", ImVec2(-1, -1)))
                {
                    std::vector<double> wheelDiametrArr;
                    std::vector<double> frontAngleArr;
                    std::vector<double> stepAngleArr;
                    std::vector<double> diametrInArr;

                    for (int i = -5; i <= 5; i++)
                    {
                        float lowestDelta = kMaxFloat;
                        ParamsToFind nearestParamsToFind = { kMaxFloat, kMaxFloat, kMaxFloat };
                        BestResult bestResult;
                        BestResultMeta meta;

                        double wheelDiametr = double(m_BestResult.Diametr) + double(i);
                        wheelDiametrArr.emplace_back(wheelDiametr);

                        GrindingWheelParams params = { wheelDiametr, m_BestResult.Width, m_BestResult.R1,
                                                       m_BestResult.R2, m_BestResult.Angle };
                        GrindingWheelProfileParams profileParams = { m_BestResult.OffsetToolCenter,
                                                                     m_BestResult.OffsetToolAxis,
                                                                     m_BestResult.RotationAngle };

                        CalculateBestResultSingle(params, profileParams, m_ToolParams, { 0.0f, 0.0f, 0.0f },
                                                  &nearestParamsToFind, &lowestDelta, &bestResult, &meta);

                        frontAngleArr.emplace_back(bestResult.FrontAngle);
                        stepAngleArr.emplace_back(bestResult.StepAngle);
                        diametrInArr.emplace_back(bestResult.DiametrIn);
                    }

                    ImPlot::SetupAxes("ToolAngle", "");
                    ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);

                    ImPlot::SetupAxisFormat(ImAxis_X1, "%g mm");

                    ImPlot::SetupAxisFormat(ImAxis_Y1, "%g mm");
                    ImPlot::SetupAxisFormat(ImAxis_Y2, "%g deg");

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Diametr In", wheelDiametrArr.data(), diametrInArr.data(), wheelDiametrArr.size());

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Front Angle", wheelDiametrArr.data(), frontAngleArr.data(),
                                     wheelDiametrArr.size());
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotLine("Step Angle", wheelDiametrArr.data(), stepAngleArr.data(), wheelDiametrArr.size());
                    DrawTooltip(wheelDiametrArr, frontAngleArr, stepAngleArr, diametrInArr, 0.25f);
                    ImPlot::EndPlot();
                }
            }
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
