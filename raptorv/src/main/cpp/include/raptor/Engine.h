#pragma once
#include "Types.h"
#include "Platform.h"
#include "Scene.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include <memory>

namespace raptor {

    struct EngineConfig {
        IFileSystem* fileSystem      = nullptr;
        bool         enableValidation = false;
        uint32_t     workerThreads    = 0;
        Quality      defaultQuality   = Quality::High;
    };

    class Engine {
    public:
        static std::unique_ptr<Engine> create(const EngineConfig& config);
        virtual ~Engine() = default;

        virtual Renderer*        getRenderer()  = 0;
        virtual ResourceManager* getResources() = 0;

        virtual std::shared_ptr<Scene> createScene() = 0;
        virtual void  setActiveScene(std::shared_ptr<Scene> scene) = 0;
        virtual Scene* getActiveScene() = 0;

        virtual void onSurfaceCreated(IWindow* window) = 0;
        virtual void onSurfaceChanged(int width, int height) = 0;
        virtual void onSurfaceDestroyed() = 0;
        virtual void tick(float deltaTime) = 0;
    };
}
