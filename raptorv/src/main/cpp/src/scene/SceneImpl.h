#pragma once
#include "raptor/Scene.h"
#include "Components.h"
#include "../physics/PhysicsWorld.h"
#include <entt/entt.hpp>
#include <filament/Engine.h>
#include <memory>

namespace raptor {

    inline EntityId   packEntity(entt::entity e) { return static_cast<EntityId>(e); }
    inline entt::entity unpackEntity(EntityId id) { return static_cast<entt::entity>(id); }

    class SceneImpl : public Scene {
    public:
        SceneImpl(filament::Engine* fEngine, filament::Scene* fScene);
        ~SceneImpl() override;

        EntityId  createEntity(const std::string& name) override;
        void      destroyEntity(EntityId id) override;
        bool      isValid(EntityId id) const override;
        void      setTransform(EntityId id, const Transform& t) override;
        Transform getTransform(EntityId id) const override;
        void      attachMesh(EntityId id, MeshHandle mesh) override;
        void      attachRigidBody(EntityId id, const RigidBodyDesc& desc) override;

        void update(float deltaTime);
        entt::registry& registry() { return m_Registry; }
        filament::Engine* filamentEngine() { return m_FEngine; }
        filament::Scene*  filamentScene()  { return m_FScene; }
        PhysicsWorld*     physics()        { return m_Physics.get(); }

    private:
        filament::Engine*             m_FEngine;
        filament::Scene*              m_FScene;
        entt::registry                m_Registry;
        std::unique_ptr<PhysicsWorld> m_Physics;
    };
}
