#pragma once
#include "raptor/Renderer.h"
#include "../scene/Components.h"
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Scene.h>
#include <utils/Entity.h>

namespace raptor {

    class SceneImpl;

    class RendererImpl : public Renderer {
    public:
        RendererImpl();
        ~RendererImpl() override;

        void     setRenderSettings(const RenderSettings& s) override;
        EntityId createCamera(const CameraDesc& desc) override;
        void     setMainCamera(EntityId camera) override;
        void     updateCamera(EntityId camera, const CameraDesc& desc) override;
        EntityId createLight(const LightDesc& desc) override;
        void     updateLight(EntityId light, const LightDesc& desc) override;

        void onSurfaceCreated(void* nativeWindow);
        void onSurfaceChanged(int w, int h);
        void onSurfaceDestroyed();
        void renderFrame();

        void              bindActiveScene(SceneImpl* scene);
        filament::Engine* engine() { return m_Engine; }

    private:
        void applySettings(const RenderSettings& s);

        filament::Engine*     m_Engine    = nullptr;
        filament::Renderer*   m_Renderer  = nullptr;
        filament::SwapChain*  m_SwapChain = nullptr;
        filament::View*       m_View      = nullptr;

        SceneImpl*            m_ActiveScene = nullptr;

        utils::Entity         m_DefaultCameraEntity;
        filament::Camera*     m_DefaultCamera = nullptr;
        EntityId              m_MainCameraId  = INVALID_ENTITY;

        int                   m_Width  = 0;
        int                   m_Height = 0;
        RenderSettings        m_Settings;
    };
}
