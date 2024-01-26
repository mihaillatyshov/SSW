#include "CustomGui.h"

#include "Engine/Core/Application.h"

#include <GLFW/glfw3.h>

namespace LM
{

    void Gui::NewFrame() { s_NeedHideCursor = false; }

    void Gui::EndFrame() { HideCursorOnEdit(); }

    void Gui::HideCursorOnEdit()
    {
        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        if (s_NeedHideCursor)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    void Gui::CheckNeedHideCursor()
    {
        if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            s_NeedHideCursor = true;
        }
    }

    bool Gui::DragFloat(const char* _Label, float* _Val, float _ValSpeed, float _ValMin, float _ValMax,
                        const char* _Format, ImGuiSliderFlags _Flags)
    {

        ImGui::Text(_Label);

        Gui::PropsTableNextColumn();

        float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
        float secondColWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(secondColWidth - itemSpacing);

        bool result =
            ImGui::DragFloat(("##" + std::string(_Label)).c_str(), _Val, _ValSpeed, _ValMin, _ValMax, _Format, _Flags);
        CheckNeedHideCursor();

        Gui::PropsTableNextColumn();

        return result;
    }

    bool Gui::DragFloatStep(const char* _Label, float* _ValLeft, float* _ValRight, int* _ValStep, float _ValSpeed,
                            float _ValMin, float _ValMax, const char* _Format, ImGuiSliderFlags _Flags)
    {
        ImGui::Text(_Label);

        Gui::PropsTableNextColumn();

        float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
        float secondColWidth = ImGui::GetContentRegionAvail().x;
        float inputSize = secondColWidth / 3.0f - itemSpacing;

        ImGui::SetNextItemWidth(inputSize);
        bool result1 = ImGui::DragFloat(("##" + std::string(_Label) + "Left").c_str(), _ValLeft, _ValSpeed, _ValMin,
                                        _ValMax, _Format, _Flags);
        CheckNeedHideCursor();
        ImGui::SameLine();

        ImGui::SetNextItemWidth(inputSize);
        bool result2 = ImGui::DragFloat(("##" + std::string(_Label) + "Right").c_str(), _ValRight, _ValSpeed, _ValMin,
                                        _ValMax, _Format, _Flags);
        CheckNeedHideCursor();
        ImGui::SameLine();

        ImGui::SetNextItemWidth(inputSize);
        bool result3 =
            ImGui::DragInt(("##" + std::string(_Label) + "Step").c_str(), _ValStep, kStepSpeed, kMinStep, kMaxStep);
        CheckNeedHideCursor();

        Gui::PropsTableNextColumn();

        return result1 || result2 || result3;
    }

    bool Gui::BeginPropsTable(const char* _StrId)
    {
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                       ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersInnerV |
                                       ImGuiTableFlags_SizingFixedFit;

        bool result = ImGui::BeginTable("_StrId", 2, flags);

        if (result)
        {
            ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Param", ImGuiTableColumnFlags_WidthStretch);

            PropsTableNextColumn();
        }

        return result;
    }

    bool Gui::PropsTableNextColumn() { return ImGui::TableNextColumn(); }

    void Gui::EndPropsTable() { ImGui::EndTable(); }

    void Gui::AlignForWidth(float _Width, float _Alignment)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        float avail = ImGui::GetContentRegionAvail().x;
        float off = (avail - _Width) * _Alignment;
        if (off > 0.0f)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        }
    }

}    // namespace LM
