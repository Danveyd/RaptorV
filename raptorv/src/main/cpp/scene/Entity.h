#pragma once
#include "Scene.h"
#include <entt/entt.hpp>

namespace raptor {
    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene)
                : m_EntityHandle(handle), m_Scene(scene) {}

        template<typename T, typename... Args>
        T& addComponent(Args&&... args) {
            return m_Scene->getRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& getComponent() {
            return m_Scene->getRegistry().get<T>(m_EntityHandle);
        }

        template<typename T>
        bool hasComponent() {
            return m_Scene->getRegistry().all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void removeComponent() {
            m_Scene->getRegistry().remove<T>(m_EntityHandle);
        }

        entt::entity getHandle() const { return m_EntityHandle; }

    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };
}
