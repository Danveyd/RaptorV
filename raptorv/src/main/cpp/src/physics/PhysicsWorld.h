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

        uint32_t createRigidBody(const Vec3& position, int type, float arg1, float arg2, float arg3, float mass);
        Vec3     getBodyPosition(uint32_t bodyId) const;
        Vec3     getBodyRotationEuler(uint32_t bodyId) const;

    private:
        JPH::PhysicsSystem*       m_System    = nullptr;
        JPH::JobSystemThreadPool* m_Jobs      = nullptr;
        JPH::TempAllocatorImpl*   m_TempAlloc = nullptr;
    };
}
