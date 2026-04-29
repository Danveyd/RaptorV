#pragma once
#include "raptor/ResourceManager.h"
#include "raptor/Platform.h"
#include <filament/Engine.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/TextureProvider.h>
#include <utils/NameComponentManager.h>
#include <unordered_map>
#include <string>
#include <memory>

namespace raptor {

    class ResourceManagerImpl : public ResourceManager {
    public:
        ResourceManagerImpl(filament::Engine* engine, IFileSystem* fs);
        ~ResourceManagerImpl() override;

        MeshHandle loadMesh(const std::string& path) override;
        void       unloadMesh(MeshHandle handle) override;
        void       unloadAll() override;

        static utils::Entity rootFromHandle(MeshHandle h);
        static void addAssetToScene(MeshHandle h, filament::Scene* scene);

    private:
        filament::Engine*                       m_Engine;
        IFileSystem*                            m_FS;
        utils::NameComponentManager*            m_Names           = nullptr;
        filament::gltfio::MaterialProvider*     m_MaterialProvider= nullptr;
        filament::gltfio::AssetLoader*          m_AssetLoader     = nullptr;
        filament::gltfio::ResourceLoader*       m_ResourceLoader  = nullptr;
        filament::gltfio::TextureProvider*      m_StbProvider     = nullptr;

        struct Entry {
            filament::gltfio::FilamentAsset* asset = nullptr;
            std::string path;
        };
        std::unordered_map<MeshHandle, Entry> m_Meshes;
        std::unordered_map<std::string, MeshHandle> m_PathToHandle;
        MeshHandle m_NextHandle = 1;
    };
}
