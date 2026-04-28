#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include <filament/Engine.h>
#include <filament/TransformManager.h>
#include "../physics/PhysicsWorld.h"

namespace raptor {

    Scene::Scene(filament::Engine* engine) : m_Engine(engine) {
        m_PhysicsWorld = std::make_unique<PhysicsWorld>();
    }
    Scene::~Scene() {}

    Entity Scene::createEntity(const std::string& name) {
        entt::entity handle = m_Registry.create();
        Entity entity(handle, this);
        entity.addComponent<TagComponent>(name);
        entity.addComponent<TransformComponent>();
        return entity;
    }

    void Scene::destroyEntity(Entity entity) {
        m_Registry.destroy(entity.getHandle());
    }

    void Scene::update(float deltaTime) {
        m_PhysicsWorld->update(deltaTime);

        auto physicsView = m_Registry.view<TransformComponent, RigidBodyComponent>();
        for (auto[entity, transform, rigidBody] : physicsView.each()) {
            transform.position = m_PhysicsWorld->getBodyPosition(rigidBody.bodyID);
            transform.rotation = m_PhysicsWorld->getBodyRotationEuler(rigidBody.bodyID);
        }

        auto& tcm = m_Engine->getTransformManager();
        auto renderView = m_Registry.view<TransformComponent, MeshComponent>();

        for (auto[entity, transform, mesh] : renderView.each()) {
            auto filInstance = tcm.getInstance(mesh.filamentEntity);
            if (filInstance.isValid()) {
                tcm.setTransform(filInstance, transform.getMatrix());
            }
        }
    }
}
