#include "PhysicsWorld.h"
#include <android/log.h>
#include <thread>
#include <algorithm>

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

namespace {

    namespace Layers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING     = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    }
    namespace BPLayers {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr uint32_t NUM_LAYERS = 2;
    }

    class BPLayerInterface final : public JPH::BroadPhaseLayerInterface {
    public:
        uint32_t GetNumBroadPhaseLayers() const override { return BPLayers::NUM_LAYERS; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer l) const override {
            return l == Layers::NON_MOVING ? BPLayers::NON_MOVING : BPLayers::MOVING;
        }
    };

    class ObjVsBP final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer a, JPH::BroadPhaseLayer b) const override {
            if (a == Layers::NON_MOVING) return b == BPLayers::MOVING;
            return true;
        }
    };

    class ObjVsObj final : public JPH::ObjectLayerPairFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
            if (a == Layers::NON_MOVING && b == Layers::NON_MOVING) return false;
            return true;
        }
    };

    static BPLayerInterface s_BP;
    static ObjVsBP          s_ObjVsBP;
    static ObjVsObj         s_ObjVsObj;
    static bool             s_JoltInitialized = false;
}

namespace raptor {

    PhysicsWorld::PhysicsWorld() {
        if (!s_JoltInitialized) {
            JPH::RegisterDefaultAllocator();
            JPH::Factory::sInstance = new JPH::Factory();
            JPH::RegisterTypes();
            s_JoltInitialized = true;
            LOGI("Jolt initialized");
        }

        m_TempAlloc = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
        unsigned hw = std::thread::hardware_concurrency();
        unsigned threads = std::max(1u, hw > 1 ? hw - 1 : 1);
        m_Jobs = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threads);

        m_System = new JPH::PhysicsSystem();
        m_System->Init(1024, 0, 1024, 1024, s_BP, s_ObjVsBP, s_ObjVsObj);
        m_System->SetGravity(JPH::Vec3(0, -9.81f, 0));
    }

    PhysicsWorld::~PhysicsWorld() {
        delete m_System;
        delete m_Jobs;
        delete m_TempAlloc;
    }

    void PhysicsWorld::update(float dt) {
        if (dt <= 0.0f) return;
        m_System->Update(dt, 1, m_TempAlloc, m_Jobs);
    }

    uint32_t PhysicsWorld::createBoxBody(const Vec3& pos, const Vec3& he, float mass) {
        JPH::BodyInterface& bi = m_System->GetBodyInterface();
        JPH::BoxShapeSettings boxShape(JPH::Vec3(he.x, he.y, he.z));
        auto shape = boxShape.Create().Get();

        bool dynamic = mass > 0.0f;
        JPH::BodyCreationSettings settings(
                shape,
                JPH::RVec3(pos.x, pos.y, pos.z),
                JPH::Quat::sIdentity(),
                dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                dynamic ? Layers::MOVING : Layers::NON_MOVING);

        JPH::Body* body = bi.CreateBody(settings);
        bi.AddBody(body->GetID(), JPH::EActivation::Activate);
        return body->GetID().GetIndexAndSequenceNumber();
    }

    Vec3 PhysicsWorld::getBodyPosition(uint32_t id) const {
        JPH::RVec3 p = m_System->GetBodyInterface().GetPosition(JPH::BodyID(id));
        return { (float)p.GetX(), (float)p.GetY(), (float)p.GetZ() };
    }

    Vec3 PhysicsWorld::getBodyRotationEuler(uint32_t id) const {
        JPH::Quat q = m_System->GetBodyInterface().GetRotation(JPH::BodyID(id));
        JPH::Vec3 e = q.GetEulerAngles();
        return { e.GetX(), e.GetY(), e.GetZ() };
    }
}
