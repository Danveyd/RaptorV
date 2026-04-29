#include "RendererImpl.h"
#include "../scene/SceneImpl.h"
#include <filament/Viewport.h>
#include <filament/LightManager.h>
#include <utils/EntityManager.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorRenderer", __VA_ARGS__)

namespace raptor {

    RendererImpl::RendererImpl() {
        LOGI("Creating Filament engine");
        m_Engine   = filament::Engine::create();
        m_Renderer = m_Engine->createRenderer();
        m_View     = m_Engine->createView();

        m_DefaultCameraEntity = utils::EntityManager::get().create();
        m_DefaultCamera = m_Engine->createCamera(m_DefaultCameraEntity);
        m_View->setCamera(m_DefaultCamera);

        applySettings(m_Settings);
    }

    RendererImpl::~RendererImpl() {
        if (!m_Engine) return;
        if (m_SwapChain) m_Engine->destroy(m_SwapChain);
        m_Engine->destroyCameraComponent(m_DefaultCameraEntity);
        utils::EntityManager::get().destroy(m_DefaultCameraEntity);
        m_Engine->destroy(m_View);
        m_Engine->destroy(m_Renderer);
        filament::Engine::destroy(&m_Engine);
    }

    void RendererImpl::applySettings(const RenderSettings& s) {
        m_Settings = s;

        filament::Renderer::ClearOptions clear;
        clear.clear = true;
        clear.clearColor = { s.clearColor.x, s.clearColor.y, s.clearColor.z, s.clearColor.w };
        m_Renderer->setClearOptions(clear);

        m_View->setAntiAliasing(s.fxaa ? filament::View::AntiAliasing::FXAA
                                       : filament::View::AntiAliasing::NONE);

        filament::View::BloomOptions bloom;
        bloom.enabled = s.bloom;
        m_View->setBloomOptions(bloom);

        filament::View::AmbientOcclusionOptions ao;
        ao.enabled = s.ssao;
        m_View->setAmbientOcclusionOptions(ao);

        filament::View::DynamicResolutionOptions dyn;
        dyn.enabled  = s.dynamicResolution;
        dyn.minScale = filament::math::float2{ s.minScale };
        dyn.maxScale = filament::math::float2{ s.maxScale };
        m_View->setDynamicResolutionOptions(dyn);

        filament::View::TemporalAntiAliasingOptions taa;
        taa.enabled = s.taa;
        m_View->setTemporalAntiAliasingOptions(taa);

        m_View->setPostProcessingEnabled(true);
    }

    void RendererImpl::setRenderSettings(const RenderSettings& s) { applySettings(s); }

    void RendererImpl::bindActiveScene(SceneImpl* scene) {
        m_ActiveScene = scene;
        if (scene) m_View->setScene(scene->filamentScene());
        else       m_View->setScene(nullptr);
    }

    EntityId RendererImpl::createCamera(const CameraDesc& desc) {
        if (!m_ActiveScene) return INVALID_ENTITY;

        EntityId id = m_ActiveScene->createEntity("Camera");
        entt::entity e = unpackEntity(id);

        utils::Entity fe = utils::EntityManager::get().create();
        filament::Camera* cam = m_Engine->createCamera(fe);

        if (m_Width > 0 && m_Height > 0) {
            double ratio = double(m_Width) / m_Height;
            cam->setProjection(desc.fovDegrees, ratio, desc.nearPlane, desc.farPlane,
                               filament::Camera::Fov::VERTICAL);
        }
        cam->lookAt({desc.position.x, desc.position.y, desc.position.z},
                    {desc.target.x,   desc.target.y,   desc.target.z},
                    {desc.up.x,       desc.up.y,       desc.up.z});

        m_ActiveScene->registry().emplace<components::CameraC>(e, components::CameraC{fe});

        if (m_MainCameraId == INVALID_ENTITY) setMainCamera(id);
        return id;
    }

    void RendererImpl::setMainCamera(EntityId camera) {
        if (!m_ActiveScene) return;
        entt::entity e = unpackEntity(camera);
        auto* cc = m_ActiveScene->registry().try_get<components::CameraC>(e);
        if (!cc) return;

        m_MainCameraId = camera;
        filament::Camera& fcam = *m_Engine->getCameraComponent(cc->filamentEntity);
        m_View->setCamera(&fcam);
    }

    void RendererImpl::updateCamera(EntityId camera, const CameraDesc& desc) {
        if (!m_ActiveScene) return;
        entt::entity e = unpackEntity(camera);
        auto* cc = m_ActiveScene->registry().try_get<components::CameraC>(e);
        if (!cc) return;

        filament::Camera* fcam = m_Engine->getCameraComponent(cc->filamentEntity);
        if (!fcam) return;

        if (m_Width > 0 && m_Height > 0) {
            double ratio = double(m_Width) / m_Height;
            fcam->setProjection(desc.fovDegrees, ratio, desc.nearPlane, desc.farPlane,
                                filament::Camera::Fov::VERTICAL);
        }
        fcam->lookAt({desc.position.x, desc.position.y, desc.position.z},
                     {desc.target.x,   desc.target.y,   desc.target.z},
                     {desc.up.x,       desc.up.y,       desc.up.z});
    }

    EntityId RendererImpl::createLight(const LightDesc& desc) {
        if (!m_ActiveScene) return INVALID_ENTITY;

        EntityId id = m_ActiveScene->createEntity("Light");
        entt::entity e = unpackEntity(id);

        utils::Entity fe = utils::EntityManager::get().create();
        filament::LightManager::Type type = filament::LightManager::Type::DIRECTIONAL;
        switch (desc.type) {
            case LightType::Directional: type = filament::LightManager::Type::DIRECTIONAL; break;
            case LightType::Point:       type = filament::LightManager::Type::POINT;       break;
            case LightType::Spot:        type = filament::LightManager::Type::SPOT;        break;
        }

        filament::LightManager::Builder(type)
                .color({desc.color.x, desc.color.y, desc.color.z})
                .intensity(desc.intensity)
                .direction({desc.direction.x, desc.direction.y, desc.direction.z})
                .position({desc.position.x, desc.position.y, desc.position.z})
                .falloff(desc.falloffRadius)
                .castShadows(desc.castShadows)
                .build(*m_Engine, fe);

        m_ActiveScene->filamentScene()->addEntity(fe);
        m_ActiveScene->registry().emplace<components::LightC>(e, components::LightC{fe});
        return id;
    }

    void RendererImpl::updateLight(EntityId light, const LightDesc& desc) {
        if (!m_ActiveScene) return;
        entt::entity e = unpackEntity(light);
        auto* lc = m_ActiveScene->registry().try_get<components::LightC>(e);
        if (!lc) return;

        auto& lm = m_Engine->getLightManager();
        auto fLight = lm.getInstance(lc->filamentEntity);
        if (!fLight.isValid()) return;

        lm.setColor(fLight, {desc.color.x, desc.color.y, desc.color.z});
        lm.setIntensity(fLight, desc.intensity);
        lm.setDirection(fLight, {desc.direction.x, desc.direction.y, desc.direction.z});
        lm.setPosition(fLight, {desc.position.x, desc.position.y, desc.position.z});
        lm.setFalloff(fLight, desc.falloffRadius);
        lm.setShadowCaster(fLight, desc.castShadows);
    }

    void RendererImpl::onSurfaceCreated(void* nativeWindow) {
        LOGI("Surface created");
        m_SwapChain = m_Engine->createSwapChain(nativeWindow);
    }

    void RendererImpl::onSurfaceChanged(int w, int h) {
        LOGI("Surface changed: %dx%d", w, h);
        m_Width  = w;
        m_Height = h;
        m_View->setViewport({0, 0, (uint32_t)w, (uint32_t)h});

        double ratio = double(w) / h;
        m_DefaultCamera->setProjection(60.0, ratio, 0.1, 1000.0, filament::Camera::Fov::VERTICAL);
        m_DefaultCamera->lookAt({0, 2, 10}, {0, 0, 0}, {0, 1, 0});
    }

    void RendererImpl::onSurfaceDestroyed() {
        LOGI("Surface destroyed");
        if (m_SwapChain) {
            m_Engine->destroy(m_SwapChain);
            m_SwapChain = nullptr;
        }
    }

    void RendererImpl::renderFrame() {
        if (!m_SwapChain) return;
        if (m_Renderer->beginFrame(m_SwapChain)) {
            m_Renderer->render(m_View);
            m_Renderer->endFrame();
        }
    }
}
