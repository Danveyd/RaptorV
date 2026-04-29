#pragma once
#include <math/vec3.h>
#include <vector>

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

        uint32_t createBoxBody(filament::math::float3 position,
                               filament::math::float3 halfExtent,
                               float mass);

        filament::math::float3 getBodyPosition(uint32_t bodyID);
        filament::math::float3 getBodyRotationEuler(uint32_t bodyID);

    private:
        JPH::PhysicsSystem* m_PhysicsSystem = nullptr;
        JPH::JobSystemThreadPool* m_JobSystem = nullptr;
        JPH::TempAllocatorImpl* m_TempAllocator = nullptr;
    };

}
