#pragma once
#include "Types.h"
#include <string>

namespace raptor {

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual EntityId createEntity(const std::string& name = "Entity") = 0;
        virtual void     destroyEntity(EntityId id) = 0;
        virtual bool     isValid(EntityId id) const = 0;

        virtual void      setTransform(EntityId id, const Transform& t) = 0;
        virtual Transform getTransform(EntityId id) const = 0;

        virtual void attachMesh     (EntityId id, MeshHandle mesh) = 0;
        virtual void attachRigidBody(EntityId id, const RigidBodyDesc& desc) = 0;
    };
}
