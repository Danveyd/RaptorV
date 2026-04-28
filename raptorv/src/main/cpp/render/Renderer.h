#pragma once
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Scene.h>
#include <android/native_window.h>

namespace raptor {

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void onSurfaceCreated(ANativeWindow* window);
        void onSurfaceChanged(int width, int height);
        void onSurfaceDestroyed();

        void beginFrame();
        void render();
        void endFrame();

        filament::Engine* getEngine() const { return m_Engine; }
        filament::Scene* getScene() const { return m_Scene; }
        filament::View* getView() const { return m_View; }

    private:
        filament::Engine* m_Engine = nullptr;
        filament::Renderer* m_Renderer = nullptr;
        filament::SwapChain* m_SwapChain = nullptr;
        filament::View* m_View = nullptr;
        filament::Scene* m_Scene = nullptr;
        filament::Camera* m_Camera = nullptr;
        utils::Entity m_CameraEntity;
    };

}
