#pragma once
#include "raptor/Engine.h"
#include "../render/RendererImpl.h"
#include "../resources/ResourceManagerImpl.h"
#include "../scene/SceneImpl.h"
#include <memory>
#include <vector>

namespace raptor {

    class EngineImpl : public Engine {
    public:
        explicit EngineImpl(const EngineConfig& cfg);
        ~EngineImpl() override;

        Renderer*        getRenderer()  override { return m_Renderer.get(); }
        ResourceManager* getResources() override { return m_Resources.get(); }

        std::shared_ptr<Scene> createScene() override;
        void  setActiveScene(std::shared_ptr<Scene> scene) override;
        Scene* getActiveScene() override { return m_ActiveScene.get(); }

        void onSurfaceCreated(IWindow* window) override;
        void onSurfaceChanged(int w, int h) override;
        void onSurfaceDestroyed() override;
        void tick(float dt) override;

    private:
        EngineConfig                       m_Config;
        std::unique_ptr<RendererImpl>      m_Renderer;
        std::unique_ptr<ResourceManagerImpl> m_Resources;
        std::shared_ptr<SceneImpl>         m_ActiveScene;
    };
}
