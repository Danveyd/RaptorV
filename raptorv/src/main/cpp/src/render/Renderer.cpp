#include "RendererImpl.h"
#include "../scene/SceneImpl.h"
#include <filament/Viewport.h>
#include <filament/LightManager.h>
#include <utils/EntityManager.h>
#include <android/log.h>
#include <algorithm>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorRenderer", __VA_ARGS__)

namespace raptor {

    RendererImpl::RendererImpl() {
        LOGI("Creating Filament engine with Vulkan backend");

        m_Engine = filament::Engine::Builder()
                .backend(filament::Engine::Backend::VULKAN)
                .build();

        if (!m_Engine) {
            LOGI("Vulkan unavailable, falling back to OpenGL");
            m_Engine = filament::Engine::create(filament::Engine::Backend::OPENGL);
        }

        /*LOGI("Creating Filament engine with OpenGL backend");

        m_Engine = filament::Engine::create(filament::Engine::Backend::OPENGL);*/

        m_Renderer = m_Engine->createRenderer();
        m_View     = m_Engine->createView();

        m_DefaultCameraEntity = utils::EntityManager::get().create();
        m_DefaultCamera = m_Engine->createCamera(m_DefaultCameraEntity);
        m_View->setCamera(m_DefaultCamera);

        m_View->setPostProcessingEnabled(true);
    }

    RendererImpl::~RendererImpl() {
        if (!m_Engine) return;

        for (auto* view : m_OffscreenViews) {
            m_Engine->destroy(view);
        }
        m_OffscreenViews.clear();

        if (m_SwapChain) m_Engine->destroy(m_SwapChain);
        m_Engine->destroyCameraComponent(m_DefaultCameraEntity);
        utils::EntityManager::get().destroy(m_DefaultCameraEntity);
        m_Engine->destroy(m_View);
        m_Engine->destroy(m_Renderer);
        filament::Engine::destroy(&m_Engine);
    }

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

        m_ActiveScene->registry().emplace<components::CameraC>(e, components::CameraC{fe});

        updateCamera(id, desc);

        if (m_MainCameraId == INVALID_ENTITY) setMainCamera(id);
        return id;
    }

    void RendererImpl::setMainCamera(EntityId camera) {
        if (!m_ActiveScene) return;
        entt::entity e = unpackEntity(camera);
        auto* cc = m_ActiveScene->registry().try_get<components::CameraC>(e);
        if (!cc) return;

        m_MainCameraId = camera;
        filament::Camera* fcam = m_Engine->getCameraComponent(cc->filamentEntity);

        if (fcam) {
            m_View->setCamera(fcam);
        }
    }

    void RendererImpl::updateCamera(EntityId camera, const CameraDesc& desc) {
        if (!m_ActiveScene) return;
        entt::entity e = unpackEntity(camera);
        auto* lc = m_ActiveScene->registry().try_get<components::CameraC>(e);
        if (!lc) return;

        filament::Camera* fcam = m_Engine->getCameraComponent(lc->filamentEntity);
        if (!fcam) return;

        double ratio = 1.0;
        if (m_Width > 0 && m_Height > 0) {
            ratio = double(m_Width) / m_Height;
        }

        if (desc.projectionType == ProjectionType::Orthographic) {
            fcam->setProjection(filament::Camera::Projection::ORTHO,
                                desc.orthoLeft, desc.orthoRight,
                                desc.orthoBottom, desc.orthoTop,
                                desc.nearPlane, desc.farPlane);
        } else if (desc.focalLength > 0.0f) {
            fcam->setLensProjection(desc.focalLength, ratio, desc.nearPlane, desc.farPlane);
        } else {
            fcam->setProjection(desc.fovDegrees, ratio, desc.nearPlane, desc.farPlane,
                                filament::Camera::Fov::VERTICAL);
        }

        fcam->setExposure(desc.aperture, 1.0f / desc.shutterSpeed, desc.sensitivityISO);

        fcam->setFocusDistance(desc.focusDistance);
        fcam->setShift(filament::math::double2{desc.shiftX, desc.shiftY});

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
            case LightType::Sun:         type = filament::LightManager::Type::SUN;         break;
        }

        auto builder = filament::LightManager::Builder(type)
                .color({desc.color.x, desc.color.y, desc.color.z})
                .intensity(desc.intensity)
                .direction({desc.direction.x, desc.direction.y, desc.direction.z})
                .position({desc.position.x, desc.position.y, desc.position.z})
                .falloff(desc.falloffRadius)
                .castShadows(desc.castShadows);

        if (desc.type == LightType::Spot) {
            builder.spotLightCone(desc.innerConeAngle, desc.outerConeAngle);
        }

        if (desc.type == LightType::Sun) {
            builder.sunAngularRadius(desc.sunAngularRadius);
            builder.sunHaloSize(desc.sunHaloSize);
            builder.sunHaloFalloff(desc.sunHaloFalloff);
        }

        if (desc.castShadows) {
            filament::LightManager::ShadowOptions shadowOpts;
            shadowOpts.mapSize = desc.shadowMapSize;
            shadowOpts.shadowCascades = desc.shadowCascades;
            shadowOpts.constantBias = desc.shadowConstantBias;
            shadowOpts.normalBias = desc.shadowNormalBias;
            shadowOpts.screenSpaceContactShadows = desc.enableContactShadows;

            shadowOpts.cascadeSplitPositions[0] = desc.cascadeSplit1;
            shadowOpts.cascadeSplitPositions[1] = desc.cascadeSplit2;
            shadowOpts.cascadeSplitPositions[2] = desc.cascadeSplit3;

            builder.shadowOptions(shadowOpts);
        }

        builder.build(*m_Engine, fe);

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

        if (desc.type == LightType::Spot) {
            lm.setSpotLightCone(fLight, desc.innerConeAngle, desc.outerConeAngle);
        }

        if (desc.type == LightType::Sun) {
            lm.setSunAngularRadius(fLight, desc.sunAngularRadius);
            lm.setSunHaloSize(fLight, desc.sunHaloSize);
            lm.setSunHaloFalloff(fLight, desc.sunHaloFalloff);
        }

        if (desc.castShadows) {
            filament::LightManager::ShadowOptions shadowOpts;
            shadowOpts.mapSize = desc.shadowMapSize;
            shadowOpts.shadowCascades = desc.shadowCascades;
            shadowOpts.constantBias = desc.shadowConstantBias;
            shadowOpts.normalBias = desc.shadowNormalBias;
            shadowOpts.screenSpaceContactShadows = desc.enableContactShadows;

            shadowOpts.cascadeSplitPositions[0] = desc.cascadeSplit1;
            shadowOpts.cascadeSplitPositions[1] = desc.cascadeSplit2;
            shadowOpts.cascadeSplitPositions[2] = desc.cascadeSplit3;

            lm.setShadowOptions(fLight, shadowOpts);
        }
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

        if (m_MainCameraId != INVALID_ENTITY && m_ActiveScene) {
            entt::entity e = unpackEntity(m_MainCameraId);
            auto* cc = m_ActiveScene->registry().try_get<components::CameraC>(e);
            if (cc) {
                filament::Camera* fcam = m_Engine->getCameraComponent(cc->filamentEntity);
                if (fcam) {
                    double ratio = double(w) / h;
                    fcam->setProjection(60.0, ratio, 0.1, 1000.0, filament::Camera::Fov::VERTICAL);
                }
            }
        }
    }

    void RendererImpl::onSurfaceDestroyed() {
        LOGI("Surface destroyed");
        if (m_SwapChain) {
            m_Engine->destroy(m_SwapChain);
            m_SwapChain = nullptr;
        }
        if (m_Engine) {
            m_Engine->flushAndWait();
        }
    }

    void RendererImpl::renderFrame() {
        if (!m_SwapChain) return;
        if (m_Renderer->beginFrame(m_SwapChain)) {
            filament::Renderer::ClearOptions mainClear;
            mainClear.clear = true;
            mainClear.clearColor = {0.0f, 0.0f, 0.02f, 1.0f};
            m_Renderer->setClearOptions(mainClear);

            if (m_View) {
                filament::Camera* camPtr = &m_View->getCamera();
            }

            m_Renderer->render(m_View);

            m_Renderer->endFrame();
        }
    }
}
