#pragma once
#include "entt/entt.hpp"
#include <android/native_window.h>
#include <android/asset_manager.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Scene.h>
#include <filament/LightManager.h>
#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <filament/IndirectLight.h>
#include <filament/MaterialInstance.h>

#include <math/vec3.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/materials/uberarchive.h>
#include <gltfio/TextureProvider.h>

struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };

class RaptorEngine {
public:
    void init(AAssetManager* assetManager);
    void update(float deltaTime);
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceDestroyed();

private:
    entt::registry registry;
    ANativeWindow* nativeWindow = nullptr;
    AAssetManager* assetMgr = nullptr;

    filament::Engine* filEngine = nullptr;
    filament::Renderer* filRenderer = nullptr;
    filament::SwapChain* filSwapChain = nullptr;
    filament::View* filView = nullptr;
    filament::Camera* filCamera = nullptr;
    filament::Scene* filScene = nullptr;

    utils::NameComponentManager* nameManager = nullptr;
    filament::gltfio::MaterialProvider* materialProvider = nullptr;
    filament::gltfio::AssetLoader* assetLoader = nullptr;
    filament::gltfio::ResourceLoader* resourceLoader = nullptr;
    filament::gltfio::FilamentAsset* filamentAsset = nullptr;
    filament::gltfio::TextureProvider* stbProvider = nullptr;
};