#include "ResourceManagerImpl.h"
#include <gltfio/materials/uberarchive.h>
#include <filament/Scene.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaptorResources", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RaptorResources", __VA_ARGS__)

namespace raptor {

    static ResourceManagerImpl* g_Instance = nullptr;

    ResourceManagerImpl::ResourceManagerImpl(filament::Engine* engine, IFileSystem* fs)
            : m_Engine(engine), m_FS(fs) {
        g_Instance = this;

        m_Names = new utils::NameComponentManager(utils::EntityManager::get());
        m_MaterialProvider = filament::gltfio::createUbershaderProvider(
                m_Engine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

        filament::gltfio::AssetConfiguration ac{};
        ac.engine        = m_Engine;
        ac.materials     = m_MaterialProvider;
        ac.names         = m_Names;
        ac.entities      = &utils::EntityManager::get();
        m_AssetLoader = filament::gltfio::AssetLoader::create(ac);

        filament::gltfio::ResourceConfiguration rc{};
        rc.engine = m_Engine;
        rc.normalizeSkinningWeights = true;
        m_ResourceLoader = new filament::gltfio::ResourceLoader(rc);

        m_StbProvider = filament::gltfio::createStbProvider(m_Engine);
        m_ResourceLoader->addTextureProvider("image/png",  m_StbProvider);
        m_ResourceLoader->addTextureProvider("image/jpeg", m_StbProvider);
    }

    ResourceManagerImpl::~ResourceManagerImpl() {
        unloadAll();
        delete m_ResourceLoader;
        delete m_StbProvider;
        if (m_AssetLoader) filament::gltfio::AssetLoader::destroy(&m_AssetLoader);
        if (m_MaterialProvider) delete m_MaterialProvider;
        delete m_Names;
        if (g_Instance == this) g_Instance = nullptr;
    }

    MeshHandle ResourceManagerImpl::loadMesh(const std::string& path) {
        if (auto it = m_PathToHandle.find(path); it != m_PathToHandle.end()) {
            return it->second;
        }

        if (!m_FS) { LOGE("No filesystem set"); return INVALID_MESH; }
        auto bytes = m_FS->readFile(path);
        if (bytes.empty()) { LOGE("Failed to read %s", path.c_str()); return INVALID_MESH; }

        auto* fasset = m_AssetLoader->createAsset(bytes.data(), bytes.size());
        if (!fasset) { LOGE("createAsset failed for %s", path.c_str()); return INVALID_MESH; }

        m_ResourceLoader->loadResources(fasset);
        fasset->releaseSourceData();

        MeshHandle h = m_NextHandle++;
        m_Meshes[h] = { fasset, path };
        m_PathToHandle[path] = h;
        LOGI("Loaded mesh %s as handle %llu", path.c_str(), (unsigned long long)h);
        return h;
    }

    void ResourceManagerImpl::unloadMesh(MeshHandle handle) {
        auto it = m_Meshes.find(handle);
        if (it == m_Meshes.end()) return;
        m_PathToHandle.erase(it->second.path);
        m_AssetLoader->destroyAsset(it->second.asset);
        m_Meshes.erase(it);
    }

    void ResourceManagerImpl::unloadAll() {
        for (auto& [h, entry] : m_Meshes) {
            m_AssetLoader->destroyAsset(entry.asset);
        }
        m_Meshes.clear();
        m_PathToHandle.clear();
    }

    utils::Entity ResourceManagerImpl::rootFromHandle(MeshHandle h) {
        if (!g_Instance) return {};
        auto it = g_Instance->m_Meshes.find(h);
        if (it == g_Instance->m_Meshes.end()) return {};
        return it->second.asset->getRoot();
    }

    void ResourceManagerImpl::addAssetToScene(MeshHandle h, filament::Scene* scene) {
        if (!g_Instance || !scene) return;
        auto it = g_Instance->m_Meshes.find(h);
        if (it != g_Instance->m_Meshes.end()) {
            scene->addEntities(it->second.asset->getEntities(), it->second.asset->getEntityCount());
        }
    }
}
