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

    enum class ToneMapping { Linear = 0, ACES = 1, Filmic = 2 };

    enum class ProjectionType { Perspective = 0, Orthographic = 1 };

    struct CameraDesc {
        ProjectionType projectionType = ProjectionType::Perspective;

        float fovDegrees    = 60.0f;
        float focalLength   = 0.0f;
        float nearPlane     = 0.1f;
        float farPlane      = 1000.0f;
        float focusDistance = 5.0f;

        Vec3  position{0, 2, 5};
        Vec3  target  {0, 0, 0};
        Vec3  up      {0, 1, 0};

        float aperture       = 16.0f;
        float shutterSpeed   = 125.0f;
        float sensitivityISO = 100.0f;

        float shiftX         = 0.0f;
        float shiftY         = 0.0f;

        float orthoLeft      = -10.0f;
        float orthoRight     = 10.0f;
        float orthoBottom    = -10.0f;
        float orthoTop       = 10.0f;
    };

    enum class LightType { Directional, Point, Spot, Sun };

    struct LightDesc {
        LightType type        = LightType::Directional;
        Vec3      color       {1, 1, 1};
        float     intensity   = 100000.0f;
        Vec3      direction   {0, -1, 0};
        Vec3      position    {0, 0, 0};
        float     falloffRadius = 10.0f;

        bool      castShadows          = true;
        bool      enableContactShadows = false;
        int       shadowMapSize        = 2048;
        int       shadowCascades       = 1;
        float     shadowConstantBias   = 0.005f;
        float     shadowNormalBias     = 0.4f;

        float     sunAngularRadius     = 0.53f;
        float     sunHaloSize          = 10.0f;
        float     sunHaloFalloff       = 80.0f;

        float     cascadeSplit1        = 0.1f;
        float     cascadeSplit2        = 0.25f;
        float     cascadeSplit3        = 0.50f;

        float     innerConeAngle = 0.5f;
        float     outerConeAngle = 0.6f;
    };

    enum class ColliderType { Box = 0, Sphere = 1, Capsule = 2 };

    struct RigidBodyDesc {
        ColliderType type = ColliderType::Box;
        Vec3  halfExtent{0.5f, 0.5f, 0.5f};
        float radius = 0.5f;
        float halfHeight = 0.5f;
        float mass = 1.0f;
    };
}
