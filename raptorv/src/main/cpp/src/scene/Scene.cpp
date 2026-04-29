#include "SceneImpl.h"
#include "../resources/ResourceManagerImpl.h"
#include <filament/TransformManager.h>
#include <filament/Scene.h>
#include <math/mat4.h>
#include <math/vec3.h>

namespace raptor {

    SceneImpl::SceneImpl(filament::Engine* fEngine, filament::Scene* fScene)
            : m_FEngine(fEngine), m_FScene(fScene) {
        m_Physics = std::make_unique<PhysicsWorld>();
    }

    SceneImpl::~SceneImpl() = default;

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
        utils::Entity root = ResourceManagerImpl::rootFromHandle(mesh);
        if (!root) return;

        entt::entity e = unpackEntity(id);
        if (!m_Registry.valid(e)) return;

        m_Registry.emplace_or_replace<components::MeshC>(e, components::MeshC{root});

        ResourceManagerImpl::addAssetToScene(mesh, m_FScene);
    }

    void SceneImpl::attachRigidBody(EntityId id, const RigidBodyDesc& desc) {
        entt::entity e = unpackEntity(id);
        if (!m_Registry.valid(e)) return;

        Vec3 pos{0, 0, 0};
        if (auto* tc = m_Registry.try_get<components::TransformC>(e)) {
            pos = tc->value.position;
        }
        uint32_t bodyId = m_Physics->createBoxBody(pos, desc.halfExtent, desc.mass);
        m_Registry.emplace_or_replace<components::RigidBodyC>(e, components::RigidBodyC{bodyId});
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
    }
}
