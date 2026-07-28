#include "SceneImpl.h"
#include "../resources/ResourceManagerImpl.h"
#include <filament/TransformManager.h>
#include <filament/Scene.h>
#include <image/Ktx1Bundle.h>
#include <ktxreader/Ktx1Reader.h>
#include <filament/IndirectLight.h>
#include <filament/Skybox.h>
#include <math/mat4.h>
#include <math/vec3.h>
#include <filament/RenderableManager.h>

namespace raptor {

    SceneImpl::SceneImpl(filament::Engine* fEngine, filament::Scene* fScene)
            : m_FEngine(fEngine), m_FScene(fScene) {
        m_Physics = std::make_unique<PhysicsWorld>();
    }

    SceneImpl::~SceneImpl() {
        m_Registry.clear();

        if (m_IndirectLight) m_FEngine->destroy(m_IndirectLight);
        if (m_IblTexture)    m_FEngine->destroy(m_IblTexture);
        if (m_Skybox)        m_FEngine->destroy(m_Skybox);
        if (m_SkyboxTexture) m_FEngine->destroy(m_SkyboxTexture);

        if (m_IblBundle)    delete m_IblBundle;
        if (m_SkyboxBundle) delete m_SkyboxBundle;
    }

    EntityId SceneImpl::createEntity(const std::string& name) {
        entt::entity e = m_Registry.create();
        m_Registry.emplace<components::Tag>(e, components::Tag{name});
        m_Registry.emplace<components::TransformC>(e, components::TransformC{});
        return packEntity(e);
    }

    void SceneImpl::destroyEntity(EntityId id) {
        entt::entity e = unpackEntity(id);
        if (!m_Registry.valid(e)) return;

        if (auto* mc = m_Registry.try_get<components::MeshC>(e)) {
            m_FScene->remove(mc->rootEntity);
        }
        m_Registry.destroy(e);
    }

    bool SceneImpl::isValid(EntityId id) const {
        return m_Registry.valid(unpackEntity(id));
    }

    void SceneImpl::setTransform(EntityId id, const Transform& t) {
        entt::entity e = unpackEntity(id);
        if (auto* tc = m_Registry.try_get<components::TransformC>(e)) {
            tc->value = t;
        }
    }

    Transform SceneImpl::getTransform(EntityId id) const {
        entt::entity e = unpackEntity(id);
        if (auto* tc = m_Registry.try_get<components::TransformC>(e)) {
            return tc->value;
        }
        return {};
    }

    void SceneImpl::attachMesh(EntityId id, MeshHandle mesh) {
        entt::entity e = unpackEntity(id);
        if (!m_Registry.valid(e)) return;

        utils::Entity root = ResourceManagerImpl::createInstance(mesh, m_FScene);
        if (!root) return;

        std::vector<filament::MaterialInstance*> instances;
        auto& rcm = m_FEngine->getRenderableManager();

        auto inst = rcm.getInstance(root);
        if (inst) {
            size_t primCount = rcm.getPrimitiveCount(inst);
            for (size_t p = 0; p < primCount; ++p) {
                auto* mi = rcm.getMaterialInstanceAt(inst, p);
                if (mi) instances.push_back(mi);
            }
        }

        m_Registry.emplace_or_replace<components::MeshC>(e, components::MeshC{root, mesh, std::move(instances)});
    }

    void SceneImpl::attachRigidBody(EntityId id, const RigidBodyDesc& desc) {
        entt::entity e = unpackEntity(id);
        if (!m_Registry.valid(e)) return;

        Vec3 pos{0, 0, 0};
        if (auto* tc = m_Registry.try_get<components::TransformC>(e)) {
            pos = tc->value.position;
        }

        int typeInt = static_cast<int>(desc.type);
        float arg1 = 0.0f, arg2 = 0.0f, arg3 = 0.0f;

        if (desc.type == ColliderType::Box) {
            arg1 = desc.halfExtent.x;
            arg2 = desc.halfExtent.y;
            arg3 = desc.halfExtent.z;
        } else if (desc.type == ColliderType::Sphere) {
            arg1 = desc.radius;
        } else if (desc.type == ColliderType::Capsule) {
            arg1 = desc.halfHeight;
            arg2 = desc.radius;
        }

        uint32_t bodyId = m_Physics->createRigidBody(pos, typeInt, arg1, arg2, arg3, desc.mass);
        m_Registry.emplace_or_replace<components::RigidBodyC>(e, components::RigidBodyC{bodyId});
    }

    void SceneImpl::setEnvironment(const uint8_t* iblData, size_t iblSize, const uint8_t* skyboxData, size_t skyboxSize) {
        if (m_IndirectLight) {
            m_FScene->setIndirectLight(nullptr);
            m_FEngine->destroy(m_IndirectLight);
            m_IndirectLight = nullptr;
        }
        if (m_IblTexture) {
            m_FEngine->destroy(m_IblTexture);
            m_IblTexture = nullptr;
        }
        if (m_IblBundle) {
            delete m_IblBundle;
            m_IblBundle = nullptr;
        }

        if (iblData && iblSize > 0) {
            m_IblBundle = new image::Ktx1Bundle(iblData, static_cast<uint32_t>(iblSize));
            filament::math::float3 harmonics[9];
            m_IblBundle->getSphericalHarmonics(harmonics);

            m_IblTexture = ktxreader::Ktx1Reader::createTexture(m_FEngine, m_IblBundle, false);
            m_IndirectLight = filament::IndirectLight::Builder()
                    .reflections(m_IblTexture)
                    .irradiance(3, harmonics)
                    .intensity(30000.0f)
                    .build(*m_FEngine);

            m_FScene->setIndirectLight(m_IndirectLight);
        }

        if (m_Skybox) {
            m_FScene->setSkybox(nullptr);
            m_FEngine->destroy(m_Skybox);
            m_Skybox = nullptr;
        }
        if (m_SkyboxTexture) {
            m_FEngine->destroy(m_SkyboxTexture);
            m_SkyboxTexture = nullptr;
        }
        if (m_SkyboxBundle) {
            delete m_SkyboxBundle;
            m_SkyboxBundle = nullptr;
        }

        if (skyboxData && skyboxSize > 0) {
            m_SkyboxBundle = new image::Ktx1Bundle(skyboxData, static_cast<uint32_t>(skyboxSize));
            m_SkyboxTexture = ktxreader::Ktx1Reader::createTexture(m_FEngine, m_SkyboxBundle, false);
            m_Skybox = filament::Skybox::Builder()
                    .environment(m_SkyboxTexture)
                    .showSun(m_ShowSun)
                    .build(*m_FEngine);

            m_FScene->setSkybox(m_Skybox);
        }
    }

    void SceneImpl::setSkyboxShowSun(bool showSun) {
        m_ShowSun = showSun;
        if (!m_FEngine || !m_FScene) return;

        if (m_SkyboxTexture) {
            filament::Skybox* newSkybox = filament::Skybox::Builder()
                    .environment(m_SkyboxTexture)
                    .showSun(m_ShowSun)
                    .build(*m_FEngine);

            auto* oldSkybox = m_FScene->getSkybox();
            if (oldSkybox) m_FEngine->destroy(oldSkybox);

            m_Skybox = newSkybox;
            m_FScene->setSkybox(m_Skybox);
        }
    }

    void SceneImpl::setIndirectLightIntensity(float intensity) {
        if (m_IndirectLight) {
            m_IndirectLight->setIntensity(intensity);
        }
    }

    void SceneImpl::setIndirectLightRotation(float rotationY) {
        if (m_IndirectLight) {
            using namespace filament::math;
            mat3f rotation = mat3f::rotation(rotationY, float3{0, 1, 0});
            m_IndirectLight->setRotation(rotation);
        }
    }

    void SceneImpl::setIndirectLightRotation3D(float rx, float ry, float rz) {
        if (m_IndirectLight) {
            using namespace filament::math;
            mat3f rotation = mat3f::rotation(ry, float3{0, 1, 0}) *
                             mat3f::rotation(rx, float3{1, 0, 0}) *
                             mat3f::rotation(rz, float3{0, 0, 1});
            m_IndirectLight->setRotation(rotation);
        }
    }

    void SceneImpl::update(float deltaTime) {
        m_Physics->update(deltaTime);

        auto physView = m_Registry.view<components::TransformC, components::RigidBodyC>();
        for (auto [e, tc, rb] : physView.each()) {
            tc.value.position = m_Physics->getBodyPosition(rb.bodyId);
            tc.value.rotation = m_Physics->getBodyRotationEuler(rb.bodyId);
        }

        auto& tcm = m_FEngine->getTransformManager();
        auto meshView = m_Registry.view<components::TransformC, components::MeshC>();
        for (auto [e, tc, mc] : meshView.each()) {
            auto inst = tcm.getInstance(mc.rootEntity);
            if (!inst.isValid()) continue;

            using namespace filament::math;
            const Transform& t = tc.value;
            mat4f S = mat4f::scaling(float3{t.scale.x, t.scale.y, t.scale.z});
            mat4f R = mat4f::rotation(t.rotation.y, float3{0, 1, 0}) *
                      mat4f::rotation(t.rotation.x, float3{1, 0, 0}) *
                      mat4f::rotation(t.rotation.z, float3{0, 0, 1});
            mat4f T = mat4f::translation(float3{t.position.x, t.position.y, t.position.z});
            tcm.setTransform(inst, T * R * S);
        }

        if (m_TransformBuffer) {
            auto view = m_Registry.view<components::TransformC>();
            size_t index = 0;

            m_TransformBuffer[index++] = static_cast<float>(view.size());

            for (auto [e, tc] : view.each()) {
                if (index + 10 >= m_TransformBufferSize) break;

                m_TransformBuffer[index++] = static_cast<float>(e);
                m_TransformBuffer[index++] = tc.value.position.x;
                m_TransformBuffer[index++] = tc.value.position.y;
                m_TransformBuffer[index++] = tc.value.position.z;
                m_TransformBuffer[index++] = tc.value.rotation.x;
                m_TransformBuffer[index++] = tc.value.rotation.y;
                m_TransformBuffer[index++] = tc.value.rotation.z;
                m_TransformBuffer[index++] = tc.value.scale.x;
                m_TransformBuffer[index++] = tc.value.scale.y;
                m_TransformBuffer[index++] = tc.value.scale.z;
            }
        }
    }
}
