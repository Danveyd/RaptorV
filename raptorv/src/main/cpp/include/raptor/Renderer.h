#pragma once
#include "Types.h"

namespace raptor {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        virtual EntityId createCamera(const CameraDesc& desc) = 0;
        virtual void     setMainCamera(EntityId camera) = 0;
        virtual void     updateCamera(EntityId camera, const CameraDesc& desc) = 0;

        virtual EntityId createLight(const LightDesc& desc) = 0;
        virtual void     updateLight(EntityId light, const LightDesc& desc) = 0;
    };
}
