#include "PhysicsWorld.h"
#include <android/log.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorPhysics", __VA_ARGS__)

namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}
namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr uint32_t NUM_LAYERS(2);
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    virtual uint32_t GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }
    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        return (inLayer == Layers::NON_MOVING) ? BroadPhaseLayers::NON_MOVING : BroadPhaseLayers::MOVING;
    }
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
        if (inLayer1 == Layers::NON_MOVING) return inLayer2 == BroadPhaseLayers::MOVING;
        return true;
    }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        if (inObject1 == Layers::NON_MOVING && inObject2 == Layers::NON_MOVING) return false;
        return true;
    }
};

namespace raptor {

    static BPLayerInterfaceImpl bpInterface;
    static ObjectVsBroadPhaseLayerFilterImpl objVsBpFilter;
    static ObjectLayerPairFilterImpl objVsObjFilter;

    PhysicsWorld::PhysicsWorld() {
        LOGI("Initializing Jolt Physics...");
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        m_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
        m_JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                                   std::thread::hardware_concurrency() - 1);

        m_PhysicsSystem = new JPH::PhysicsSystem();
        m_PhysicsSystem->Init(1024, 0, 1024, 1024, bpInterface, objVsBpFilter, objVsObjFilter);

        m_PhysicsSystem->SetGravity(JPH::Vec3(0, -9.81f, 0));
    }

    PhysicsWorld::~PhysicsWorld() {
        delete m_PhysicsSystem;
        delete m_JobSystem;
        delete m_TempAllocator;
        delete JPH::Factory::sInstance;
    }

    void PhysicsWorld::update(float deltaTime) {
        const int collisionSteps = 1;
        m_PhysicsSystem->Update(deltaTime, collisionSteps, m_TempAllocator, m_JobSystem);
    }

    uint32_t PhysicsWorld::createBoxBody(filament::math::float3 pos, filament::math::float3 halfExt, float mass) {
        JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();

        JPH::BoxShapeSettings boxShape(JPH::Vec3(halfExt.x, halfExt.y, halfExt.z));
        JPH::ShapeSettings::ShapeResult shapeResult = boxShape.Create();
        JPH::ShapeRefC shape = shapeResult.Get();

        JPH::EMotionType motionType = (mass > 0.0f) ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static;
        JPH::ObjectLayer layer = (mass > 0.0f) ? Layers::MOVING : Layers::NON_MOVING;

        JPH::BodyCreationSettings settings(shape, JPH::RVec3(pos.x, pos.y, pos.z), JPH::Quat::sIdentity(), motionType, layer);

        JPH::Body* body = bodyInterface.CreateBody(settings);
        bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);

        return body->GetID().GetIndexAndSequenceNumber();
    }

    filament::math::float3 PhysicsWorld::getBodyPosition(uint32_t bodyID) {
        JPH::BodyID id(bodyID);
        JPH::RVec3 pos = m_PhysicsSystem->GetBodyInterface().GetPosition(id);
        return { (float)pos.GetX(), (float)pos.GetY(), (float)pos.GetZ() };
    }

    filament::math::float3 PhysicsWorld::getBodyRotationEuler(uint32_t bodyID) {
        JPH::BodyID id(bodyID);
        JPH::Quat rot = m_PhysicsSystem->GetBodyInterface().GetRotation(id);
        JPH::Vec3 euler = rot.GetEulerAngles();
        return { euler.GetX(), euler.GetY(), euler.GetZ() };
    }

}
