#pragma once
#include "raptor/Platform.h"
#include <android/native_window.h>

namespace raptor {

    class AndroidWindow : public IWindow {
    public:
        AndroidWindow(ANativeWindow* window, int w, int h)
                : m_Window(window), m_Width(w), m_Height(h) {}

        void* getNativeHandle() override { return m_Window; }
        int   getWidth()  const override { return m_Width; }
        int   getHeight() const override { return m_Height; }

    private:
        ANativeWindow* m_Window;
        int m_Width;
        int m_Height;
    };
}
