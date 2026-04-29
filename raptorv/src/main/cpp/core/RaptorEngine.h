#pragma once
#include <android/native_window.h>
#include <android/asset_manager.h>
#include <memory>
#include <string>
#include <vector>

#include "../scene/Scene.h"
#include "../render/Renderer.h"

#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/MaterialProvider.h>
#include <utils/NameComponentManager.h>

class RaptorEngine {
public:
    static RaptorEngine& get() {
        static RaptorEngine instance;
        return instance;
    }

    void init(AAssetManager* assetManager);
    void update(float deltaTime);
    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceChanged(int width, int height);
    void onSurfaceDestroyed();

    uint32_t createEntity(const std::string& name);
    void destroyEntity(uint32_t entityId);
    void setPosition(uint32_t entityId, float x, float y, float z);
    void setRotation(uint32_t entityId, float x, float y, float z);
    void setScale(uint32_t entityId, float x, float y, float z);
    void addBoxCollider(uint32_t entityId, float hx, float hy, float hz, float mass);
    filament::math::float3 getPosition(uint32_t entityId);
    filament::math::float3 getRotation(uint32_t entityId);
    void loadGlTF(uint32_t entityId, const std::string& path);

    raptor::Scene* getActiveScene() { return m_ActiveScene.get(); }

private:
    RaptorEngine() = default;

    std::unique_ptr<raptor::Renderer> m_Renderer;
    std::unique_ptr<raptor::Scene> m_ActiveScene;
    AAssetManager* m_AssetManager = nullptr;

    utils::NameComponentManager* m_NameManager = nullptr;
    filament::gltfio::MaterialProvider* m_MaterialProvider = nullptr;
    filament::gltfio::AssetLoader* m_AssetLoader = nullptr;
    filament::gltfio::ResourceLoader* m_ResourceLoader = nullptr;
    std::vector<filament::gltfio::FilamentAsset*> m_Assets;
};
