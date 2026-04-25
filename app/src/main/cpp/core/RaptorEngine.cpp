#include "RaptorEngine.h"
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorV", __VA_ARGS__)

void RaptorEngine::init() {
    LOGI("RaptorEngine initialized! Welcome to C++!");

    auto entity = registry.create();
    LOGI("Created first ECS Entity with ID: %d", (int)entity);
}

void RaptorEngine::update(float deltaTime) {

}