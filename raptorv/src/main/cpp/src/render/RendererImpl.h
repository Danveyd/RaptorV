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
#include <vector>

namespace raptor {

    class SceneImpl;

    class RendererImpl : public Renderer {
    public:
        RendererImpl();
        ~RendererImpl() override;

        EntityId createCamera(const CameraDesc& desc) override;
        void     setMainCamera(EntityId camera) override;
        void     updateCamera(EntityId camera, const CameraDesc& desc) override;
        EntityId createLight(const LightDesc& desc) override;
        void     updateLight(EntityId light, const LightDesc& desc) override;

        void onSurfaceCreated(void* nativeWindow);
        void onSurfaceChanged(int w, int h);
        void onSurfaceDestroyed();
        void renderFrame();

        void addOffscreenView(filament::View* view) {
            m_OffscreenViews.push_back(view);
        }
        void removeOffscreenView(filament::View* view) {
            auto it = std::find(m_OffscreenViews.begin(), m_OffscreenViews.end(), view);
            if (it != m_OffscreenViews.end()) {
                m_OffscreenViews.erase(it);
            }
        }

        void              bindActiveScene(SceneImpl* scene);
        filament::Engine* engine() { return m_Engine; }
        filament::View*   getView() { return m_View; }
        SceneImpl* getActiveScene() const { return m_ActiveScene; }

    private:
        filament::Engine*     m_Engine    = nullptr;
        filament::Renderer*   m_Renderer  = nullptr;
        filament::SwapChain*  m_SwapChain = nullptr;
        filament::View*       m_View      = nullptr;

        SceneImpl*            m_ActiveScene = nullptr;

        utils::Entity         m_DefaultCameraEntity;
        filament::Camera*     m_DefaultCamera = nullptr;
        EntityId              m_MainCameraId  = INVALID_ENTITY;

        std::vector<filament::View*> m_OffscreenViews;

        int                   m_Width  = 0;
        int                   m_Height = 0;
    };
}
