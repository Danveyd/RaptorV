#pragma once
#include <string>
#include <vector>
#include <entt/entt.hpp>
#include <math/vec3.h>
#include <math/mat4.h>
#include <utils/Entity.h>

namespace JPH { class BodyID; }

namespace raptor {

    struct TagComponent {
        std::string name;
        TagComponent() = default;
        TagComponent(const std::string& name) : name(name) {}
    };

    struct TransformComponent {
        filament::math::float3 position = {0.0f, 0.0f, 0.0f};
        filament::math::float3 rotation = {0.0f, 0.0f, 0.0f};
        filament::math::float3 scale = {1.0f, 1.0f, 1.0f};

        filament::math::mat4f getMatrix() const {
            auto S = filament::math::mat4f::scaling(scale);

            auto R = filament::math::mat4f::rotation(rotation.y, filament::math::float3{0,1,0}) *
                     filament::math::mat4f::rotation(rotation.x, filament::math::float3{1,0,0}) *
                     filament::math::mat4f::rotation(rotation.z, filament::math::float3{0,0,1});

            auto T = filament::math::mat4f::translation(position);

            return T * R * S;
        }
    };

    struct MeshComponent {
        utils::Entity filamentEntity;

        MeshComponent() = default;
        MeshComponent(utils::Entity e) : filamentEntity(e) {}
    };

    struct RigidBodyComponent {
        uint32_t bodyID;

        RigidBodyComponent() = default;
        RigidBodyComponent(uint32_t id) : bodyID(id) {}
    };
}
