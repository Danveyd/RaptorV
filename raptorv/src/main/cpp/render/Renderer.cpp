#include "Renderer.h"
#include <filament/Viewport.h>
#include <filament/TransformManager.h>
#include <utils/EntityManager.h>
#include <filament/LightManager.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorRenderer", __VA_ARGS__)

namespace raptor {

    Renderer::Renderer() {
        LOGI("Initializing Filament Engine...");
        m_Engine = filament::Engine::create();
        m_Renderer = m_Engine->createRenderer();
        m_Scene = m_Engine->createScene();
        m_View = m_Engine->createView();

        filament::View::DynamicResolutionOptions dynRes;
        dynRes.enabled = true;
        dynRes.minScale = 0.5f;
        dynRes.maxScale = 1.0f;
        m_View->setDynamicResolutionOptions(dynRes);

        filament::Renderer::ClearOptions clearOptions;
        clearOptions.clear = true;
        clearOptions.clearColor = {0.0f, 0.0f, 0.02f, 1.0f};
        m_Renderer->setClearOptions(clearOptions);

        m_CameraEntity = utils::EntityManager::get().create();
        m_Camera = m_Engine->createCamera(m_CameraEntity);
        m_View->setCamera(m_Camera);
        m_View->setScene(m_Scene);

        m_View->setAntiAliasing(filament::View::AntiAliasing::FXAA);
        filament::View::BloomOptions bloom;
        bloom.enabled = true;
        m_View->setBloomOptions(bloom);

        m_View->setAmbientOcclusionOptions({.enabled = true});
        m_View->setPostProcessingEnabled(true);

        utils::Entity lightEntity = utils::EntityManager::get().create();
        filament::LightManager::Builder(filament::LightManager::Type::DIRECTIONAL)
                .color(filament::math::float3{1.0f, 1.0f, 0.9f})
                .intensity(100000.0f)
                .direction({0.5f, -1.0f, -0.5f})
                .castShadows(true)
                .build(*m_Engine, lightEntity);
        m_Scene->addEntity(lightEntity);
    }

    Renderer::~Renderer() {
        if (m_Engine) {
            m_Engine->destroy(m_CameraEntity);
            filament::Engine::destroy(&m_Engine);
        }
    }

    void Renderer::onSurfaceCreated(ANativeWindow* window) {
        LOGI("Surface Created!");
        m_SwapChain = m_Engine->createSwapChain(window);
    }

    void Renderer::onSurfaceChanged(int width, int height) {
        LOGI("Surface Changed: %dx%d", width, height);
        m_View->setViewport({0, 0, (uint32_t)width, (uint32_t)height});

        double ratio = double(width) / height;
        m_Camera->setProjection(45.0, ratio, 0.1, 1000.0, filament::Camera::Fov::VERTICAL);

        m_Camera->lookAt({0.0, 2.0, 10.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
    }

    void Renderer::onSurfaceDestroyed() {
        LOGI("Surface Destroyed!");
        if (m_SwapChain) {
            m_Engine->destroy(m_SwapChain);
            m_SwapChain = nullptr;
        }
    }

    void Renderer::beginFrame() {
    }

    void Renderer::render() {
        if (m_Renderer && m_SwapChain) {
            if (m_Renderer->beginFrame(m_SwapChain)) {
                m_Renderer->render(m_View);
                m_Renderer->endFrame();
            }
        }
    }

    void Renderer::endFrame() {
    }

}
