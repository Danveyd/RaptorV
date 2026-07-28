#pragma once
#include <string>
#include <vector>
#include <utils/Entity.h>
#include <filament/MaterialInstance.h>
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
        std::vector<filament::MaterialInstance*> materialInstances;
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
