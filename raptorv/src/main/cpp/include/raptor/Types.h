#pragma once
#include <cstdint>

namespace raptor {

    using EntityId   = uint64_t;
    using MeshHandle = uint64_t;
    using SceneHandle= uint64_t;

    constexpr EntityId   INVALID_ENTITY = 0;
    constexpr MeshHandle INVALID_MESH   = 0;

    struct Vec3 {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        Vec3() = default;
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    };

    struct Vec4 {
        float x = 0, y = 0, z = 0, w = 0;
    };

    struct Transform {
        Vec3 position{0, 0, 0};
        Vec3 rotation{0, 0, 0};
        Vec3 scale   {1, 1, 1};
    };

    enum class Quality { Low, Medium, High, Ultra };

    struct RenderSettings {
        bool  bloom             = true;
        bool  ssao              = true;
        bool  fxaa              = true;
        bool  taa               = false;
        bool  dynamicResolution = true;
        float minScale          = 0.5f;
        float maxScale          = 1.0f;
        Vec4  clearColor        {0.0f, 0.0f, 0.02f, 1.0f};
        Quality quality         = Quality::High;
    };

    struct CameraDesc {
        float fovDegrees = 60.0f;
        float nearPlane  = 0.1f;
        float farPlane   = 1000.0f;
        Vec3  position{0, 0, 0};
        Vec3  target  {0, 0, -1};
        Vec3  up      {0, 1, 0};
    };

    enum class LightType { Directional, Point, Spot };

    struct LightDesc {
        LightType type        = LightType::Directional;
        Vec3      color       {1, 1, 1};
        float     intensity   = 100000.0f;
        Vec3      direction   {0, -1, 0};
        Vec3      position    {0, 0, 0};
        float     falloffRadius = 10.0f;
        bool      castShadows  = true;
    };

    struct RigidBodyDesc {
        Vec3  halfExtent{0.5f, 0.5f, 0.5f};
        float mass = 1.0f;
    };
}
