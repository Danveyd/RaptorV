#pragma once
#include "Types.h"
#include <string>

namespace raptor {

    class ResourceManager {
    public:
        virtual ~ResourceManager() = default;
        virtual MeshHandle loadMesh(const std::string& path) = 0;
        virtual void       unloadMesh(MeshHandle handle) = 0;
        virtual void       unloadAll() = 0;
    };
}
