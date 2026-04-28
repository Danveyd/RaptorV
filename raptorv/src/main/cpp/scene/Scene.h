#pragma once
#include <entt/entt.hpp>
#include <string>
#include "../physics/PhysicsWorld.h"

namespace filament { class Engine; }

namespace raptor {
    class Entity;

    class Scene {
    public:
        Scene(filament::Engine* engine);
        ~Scene();

        Entity createEntity(const std::string& name = "Entity");
        void destroyEntity(Entity entity);
        void update(float deltaTime);

        entt::registry& getRegistry() { return m_Registry; }

        PhysicsWorld* getPhysicsWorld() { return m_PhysicsWorld.get(); }

    private:
        filament::Engine* m_Engine;
        entt::registry m_Registry;
        std::unique_ptr<PhysicsWorld> m_PhysicsWorld;
    };
}
