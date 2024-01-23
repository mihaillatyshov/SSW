#pragma once

#include "Engine/Core/Window.h"

struct GLFWwindow;

namespace LM
{

    class GLFWWindow : public Window
    {
    public:
        GLFWWindow(const WindowProps& _Props);
        virtual ~GLFWWindow();

        virtual uint32_t GetWidth() const override { return m_Data.Width; }
        virtual uint32_t GetHeight() const override { return m_Data.Height; }

        virtual void OnUpdate() override;

        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

        virtual void* GetNativeWindow() const override { return m_Window; }

    protected:
        bool Init();
        void SetCallbacks();

    protected:
        struct WindowData : WindowProps
        {
            WindowData(const WindowProps& _Props) : WindowProps(_Props) { }
            EventCallbackFn EventCallback;
        };

        GLFWwindow* m_Window;

        WindowData m_Data;
    };

}    // namespace LM
