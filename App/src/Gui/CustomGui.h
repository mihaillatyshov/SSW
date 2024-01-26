#pragma once

#include <imgui.h>

namespace LM
{

    class Gui
    {
    protected:
        static inline constexpr int kMinStep = 1;
        static inline constexpr int kMaxStep = 1000;
        static inline constexpr float kStepSpeed = 0.1f;

    public:
        static void NewFrame();

        static void EndFrame();

        static void HideCursorOnEdit();

        static void CheckNeedHideCursor();

        static bool DragFloat(const char* _Label, float* _Val, float _ValSpeed = 1.0f, float _ValMin = 0.0f,
                              float _ValMax = 0.0f, const char* _Format = "%.3f", ImGuiSliderFlags _Flags = 0);

        static bool DragFloatStep(const char* _Label, float* _ValLeft, float* _ValRight, int* _ValStep,
                                  float _ValSpeed = 1.0f, float _ValMin = 0.0f, float _ValMax = 0.0f,
                                  const char* _Format = "%.3f", ImGuiSliderFlags _Flags = 0);

        static bool BeginPropsTable(const char* _StrId);
        static bool PropsTableNextColumn();
        static void EndPropsTable();

        static void AlignForWidth(float _Width, float _Alignment = 0.5f);

    protected:
        static inline bool s_NeedHideCursor = false;
    };

}    // namespace LM
