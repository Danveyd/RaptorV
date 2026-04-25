#pragma once
#include "entt/entt.hpp"

class RaptorEngine {
public:
    void init();
    void update(float deltaTime);

private:
    entt::registry registry;
};