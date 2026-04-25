#include "RaptorEngine.h"
#include <android/log.h>
#include <filament/Viewport.h>
#include <filament/TransformManager.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorV", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RaptorV", __VA_ARGS__)

void RaptorEngine::init(AAssetManager* assetManager) {
    LOGI("RaptorEngine initialized!");
    assetMgr = assetManager;

    filEngine = filament::Engine::create();
    filRenderer = filEngine->createRenderer();
    filScene = filEngine->createScene();
    filView = filEngine->createView();

    filament::Renderer::ClearOptions clearOptions;
    clearOptions.clear = true;
    clearOptions.clearColor = {0.1f, 0.2f, 0.4f, 1.0f};
    filRenderer->setClearOptions(clearOptions);

    auto camEntity = utils::EntityManager::get().create();
    filCamera = filEngine->createCamera(camEntity);
    filView->setCamera(filCamera);
    filView->setScene(filScene);

    auto lightEntity = utils::EntityManager::get().create();
    filament::LightManager::Builder(filament::LightManager::Type::DIRECTIONAL)
            .color(filament::math::float3{1.0f, 1.0f, 0.9f})
            .intensity(100000.0f)
            .direction({0.5f, -1.0f, -0.5f})
            .castShadows(true)
            .build(*filEngine, lightEntity);
    filScene->addEntity(lightEntity);
    auto lightFillEntity = utils::EntityManager::get().create();
    filament::LightManager::Builder(filament::LightManager::Type::DIRECTIONAL)
            .color(filament::math::float3{0.5f, 0.7f, 1.0f})
            .intensity(2000.0f)
            .direction({-0.5f, 1.0f, 0.5f})
            .build(*filEngine, lightFillEntity);
    filScene->addEntity(lightFillEntity);
    float iblIntensity = 30000.0f;
    filament::math::float3 harmonics[9] = { {0.5f, 0.5f, 0.5f} };

    static const filament::math::float3 sh[] = {
            {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}
    };

    auto indirectLightEntity = utils::EntityManager::get().create();
    filament::IndirectLight* ibl = filament::IndirectLight::Builder()
            .irradiance(3, sh)
            .intensity(5000.0f)
            .build(*filEngine);

    filScene->setIndirectLight(ibl);

    filView->setAntiAliasing(filament::View::AntiAliasing::FXAA);

    filament::View::BloomOptions bloom;
    bloom.enabled = true;
    filView->setBloomOptions(bloom);

    filView->setAmbientOcclusionOptions({.enabled = true});

    materialProvider = filament::gltfio::createUbershaderProvider(
            filEngine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE
    );

    nameManager = new utils::NameComponentManager(utils::EntityManager::get());
    materialProvider = filament::gltfio::createUbershaderProvider(filEngine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

    filament::gltfio::AssetConfiguration assetConfig = {filEngine, materialProvider, nameManager, &utils::EntityManager::get()};
    assetLoader = filament::gltfio::AssetLoader::create(assetConfig);

    AAsset* asset = AAssetManager_open(assetMgr, "apocalypse_city__1.glb", AASSET_MODE_BUFFER);
    if (asset) {
        size_t size = AAsset_getLength(asset);
        const void* data = AAsset_getBuffer(asset);

        filamentAsset = assetLoader->createAsset((const uint8_t*)data, (uint32_t)size);

        if (filamentAsset) {
            filament::gltfio::ResourceConfiguration resConfig = {};
            resConfig.engine = filEngine;
            resConfig.normalizeSkinningWeights = true;

            resourceLoader = new filament::gltfio::ResourceLoader(resConfig);
            stbProvider = filament::gltfio::createStbProvider(filEngine);

            resourceLoader->addTextureProvider("image/png", stbProvider);
            resourceLoader->addTextureProvider("image/jpeg", stbProvider);

            resourceLoader->loadResources(filamentAsset);
            AAsset_close(asset);

            filScene->addEntities(filamentAsset->getEntities(), filamentAsset->getEntityCount());
        }
    }
}

void RaptorEngine::onSurfaceCreated(ANativeWindow* window) {
    nativeWindow = window;
    int width = ANativeWindow_getWidth(window);
    int height = ANativeWindow_getHeight(window);

    filSwapChain = filEngine->createSwapChain(nativeWindow);
    filView->setViewport({0, 0, (uint32_t)width, (uint32_t)height});

    if (filamentAsset) {
        auto box = filamentAsset->getBoundingBox();

        filament::math::float3 center = (box.min + box.max) * 0.5f;
        filament::math::float3 extent = (box.max - box.min) * 0.5f;

        float maxDim = std::max({extent.x, extent.y, extent.z});

        float cameraDistance = std::max(maxDim * 3.5f, 5.0f);

        filCamera->setProjection(45.0, double(width)/height, 0.1, cameraDistance * 10.0, filament::Camera::Fov::VERTICAL);

        filament::math::double3 eye = {
                (double)center.x,
                (double)center.y + cameraDistance * 0.5,
                (double)center.z + cameraDistance
        };
        filament::math::double3 at = {(double)center.x, (double)center.y, (double)center.z};
        filament::math::double3 up = {0.0, 1.0, 0.0};

        filCamera->lookAt(eye, at, up);

        LOGI("Camera focused. Center: %.2f %.2f %.2f, Distance: %.2f", center.x, center.y, center.z, cameraDistance);
    }
}

void RaptorEngine::update(float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for(auto[entity, pos, vel] : view.each()) {
        pos.x += vel.dx * deltaTime;
    }

    if (filRenderer && filSwapChain) {
        if (filRenderer->beginFrame(filSwapChain)) {
            filRenderer->render(filView);
            filRenderer->endFrame();
        }
    }
}

void RaptorEngine::onSurfaceDestroyed() {
    LOGI("Surface destroyed!");
    if (filSwapChain) {
        filEngine->destroy(filSwapChain);
        filSwapChain = nullptr;
    }
    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = nullptr;
    }
}