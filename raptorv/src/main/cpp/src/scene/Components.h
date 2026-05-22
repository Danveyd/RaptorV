#pragma once
#include <string>
#include <utils/Entity.h>
#include "raptor/Types.h"

namespace raptor::components {

    struct Tag {
        std::string name;
    };

    struct TransformC {
        Transform value;
    };

    struct MeshC {
        utils::Entity rootEntity;
        MeshHandle meshHandle = INVALID_MESH;
    };

    struct RigidBodyC {
        uint32_t bodyId = 0;
    };

    struct CameraC {
        utils::Entity filamentEntity;
    };

    struct LightC {
        utils::Entity filamentEntity;
    };
}
