#include "EngineImpl.h"

namespace raptor {

    std::unique_ptr<Engine> Engine::create(const EngineConfig& cfg) {
        return std::make_unique<EngineImpl>(cfg);
    }

    EngineImpl::EngineImpl(const EngineConfig& cfg) : m_Config(cfg) {
        m_Renderer  = std::make_unique<RendererImpl>();
        m_Resources = std::make_unique<ResourceManagerImpl>(m_Renderer->engine(), cfg.fileSystem);
    }

    EngineImpl::~EngineImpl() {
        m_ActiveScene.reset();
        m_Resources.reset();
        m_Renderer.reset();
    }

    std::shared_ptr<Scene> EngineImpl::createScene() {
        auto fEngine = m_Renderer->engine();
        filament::Scene* fScene = fEngine->createScene();
        return std::make_shared<SceneImpl>(fEngine, fScene);
    }

    void EngineImpl::setActiveScene(std::shared_ptr<Scene> scene) {
        m_ActiveScene = std::static_pointer_cast<SceneImpl>(scene);
        m_Renderer->bindActiveScene(m_ActiveScene.get());
    }

    void EngineImpl::onSurfaceCreated(IWindow* window) {
        m_Renderer->onSurfaceCreated(window->getNativeHandle());
    }

    void EngineImpl::onSurfaceChanged(int w, int h) {
        m_Renderer->onSurfaceChanged(w, h);
    }

    void EngineImpl::onSurfaceDestroyed() {
        m_Renderer->onSurfaceDestroyed();
    }

    void EngineImpl::tick(float dt) {
        if (m_ActiveScene) m_ActiveScene->update(dt);
        m_Renderer->renderFrame();
    }
}
