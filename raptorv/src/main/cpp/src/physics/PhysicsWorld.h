#pragma once
#include "raptor/Types.h"
#include <cstdint>

namespace JPH {
    class PhysicsSystem;
    class JobSystemThreadPool;
    class TempAllocatorImpl;
}

namespace raptor {

    class PhysicsWorld {
    public:
        PhysicsWorld();
        ~PhysicsWorld();

        void update(float deltaTime);

        uint32_t createBoxBody(const Vec3& position, const Vec3& halfExtent, float mass);
        Vec3     getBodyPosition(uint32_t bodyId) const;
        Vec3     getBodyRotationEuler(uint32_t bodyId) const;

    private:
        JPH::PhysicsSystem*       m_System    = nullptr;
        JPH::JobSystemThreadPool* m_Jobs      = nullptr;
        JPH::TempAllocatorImpl*   m_TempAlloc = nullptr;
    };
}
