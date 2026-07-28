#pragma once
#include "raptor/Scene.h"
#include "Components.h"
#include "../physics/PhysicsWorld.h"
#include <entt/entt.hpp>
#include <filament/Engine.h>
#include <memory>
#include <image/Ktx1Bundle.h>

namespace raptor {

    inline EntityId   packEntity(entt::entity e) { return static_cast<EntityId>(e) + 1; }
    inline entt::entity unpackEntity(EntityId id) { return static_cast<entt::entity>(id > 0 ? id - 1 : 0); }

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
        void setEnvironment(const uint8_t* iblData, size_t iblSize, const uint8_t* skyboxData, size_t skyboxSize);
        void setTransformBuffer(float* buffer, size_t maxSize) {
            m_TransformBuffer = buffer;
            m_TransformBufferSize = maxSize;
        }

        void setIndirectLightIntensity(float intensity) override;

        void setIndirectLightRotation(float rotationY) override;
        void setIndirectLightRotation3D(float rx, float ry, float rz);

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
        filament::IndirectLight* m_IndirectLight = nullptr;
        filament::Texture*       m_IblTexture = nullptr;
        filament::Skybox*        m_Skybox = nullptr;
        filament::Texture*       m_SkyboxTexture = nullptr;
        image::Ktx1Bundle* m_IblBundle = nullptr;
        image::Ktx1Bundle* m_SkyboxBundle = nullptr;
        float* m_TransformBuffer = nullptr;
        size_t m_TransformBufferSize = 0;
    };
}
