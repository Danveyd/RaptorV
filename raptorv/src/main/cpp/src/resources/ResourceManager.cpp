#include "ResourceManagerImpl.h"
#include <gltfio/materials/uberarchive.h>
#include <filament/Scene.h>
#include <android/log.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <utils/EntityManager.h>
#include <math/vec3.h>
#include <geometry/SurfaceOrientation.h>
#include <filament/TransformManager.h>

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
        // fasset->releaseSourceData();

        MeshHandle h = m_NextHandle++;

        Entry entry;
        entry.asset = fasset;
        entry.path = path;

        m_Meshes[h] = entry;
        m_PathToHandle[path] = h;
        LOGI("Loaded mesh %s as handle %llu", path.c_str(), (unsigned long long)h);
        return h;
    }

    MeshHandle ResourceManagerImpl::createMesh(const float* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount) {
        std::vector<filament::math::float3> positions(vertexCount);
        std::vector<filament::math::float2> uvs(vertexCount);
        std::vector<filament::math::float3> normals(vertexCount);

        for (size_t i = 0; i < vertexCount; ++i) {
            positions[i] = { vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2] };
            uvs[i]       = { vertices[i * 8 + 3], vertices[i * 8 + 4] };
            normals[i]   = { vertices[i * 8 + 5], vertices[i * 8 + 6], vertices[i * 8 + 7] };
        }

        size_t triangleCount = indexCount / 3;
        std::vector<filament::math::uint3> triangles(triangleCount);
        for (size_t i = 0; i < triangleCount; ++i) {
            triangles[i] = { indices[i * 3 + 0], indices[i * 3 + 1], indices[i * 3 + 2] };
        }

        filament::geometry::SurfaceOrientation* orientation = filament::geometry::SurfaceOrientation::Builder()
                .vertexCount(vertexCount)
                .positions(positions.data())
                .normals(normals.data())
                .uvs(uvs.data())
                .triangleCount(triangleCount)
                .triangles(triangles.data())
                .build();

        std::vector<filament::math::quatf> tangents(vertexCount);
        orientation->getQuats(tangents.data(), vertexCount);
        delete orientation;

        filament::VertexBuffer* vb = filament::VertexBuffer::Builder()
                .vertexCount(vertexCount)
                .bufferCount(4)
                .attribute(filament::VertexAttribute::POSITION, 0, filament::VertexBuffer::AttributeType::FLOAT3)
                .attribute(filament::VertexAttribute::UV0,      1, filament::VertexBuffer::AttributeType::FLOAT2)
                .attribute(filament::VertexAttribute::TANGENTS, 2, filament::VertexBuffer::AttributeType::FLOAT4)
                .attribute(filament::VertexAttribute::COLOR,    3, filament::VertexBuffer::AttributeType::UBYTE4)
                .normalized(filament::VertexAttribute::COLOR)
                .build(*m_Engine);

        auto* posData = new filament::math::float3[vertexCount];
        std::memcpy(posData, positions.data(), vertexCount * sizeof(filament::math::float3));
        vb->setBufferAt(*m_Engine, 0, filament::VertexBuffer::BufferDescriptor(posData, vertexCount * sizeof(filament::math::float3), [](void* b, size_t, void*) { delete[] static_cast<filament::math::float3*>(b); }));

        auto* uvData = new filament::math::float2[vertexCount];
        std::memcpy(uvData, uvs.data(), vertexCount * sizeof(filament::math::float2));
        vb->setBufferAt(*m_Engine, 1, filament::VertexBuffer::BufferDescriptor(uvData, vertexCount * sizeof(filament::math::float2), [](void* b, size_t, void*) { delete[] static_cast<filament::math::float2*>(b); }));

        auto* tanData = new filament::math::quatf[vertexCount];
        std::memcpy(tanData, tangents.data(), vertexCount * sizeof(filament::math::quatf));
        vb->setBufferAt(*m_Engine, 2, filament::VertexBuffer::BufferDescriptor(tanData, vertexCount * sizeof(filament::math::quatf), [](void* b, size_t, void*) { delete[] static_cast<filament::math::quatf*>(b); }));

        auto* colorData = new uint32_t[vertexCount];
        std::fill_n(colorData, vertexCount, 0xFFFFFFFF);
        vb->setBufferAt(*m_Engine, 3, filament::VertexBuffer::BufferDescriptor(colorData, vertexCount * sizeof(uint32_t), [](void* b, size_t, void*) { delete[] static_cast<uint32_t*>(b); }));

        filament::IndexBuffer* ib = filament::IndexBuffer::Builder()
                .indexCount(indexCount)
                .bufferType(filament::IndexBuffer::IndexType::UINT)
                .build(*m_Engine);

        size_t ibSize = indexCount * sizeof(uint32_t);
        uint32_t* ibData = new uint32_t[indexCount];
        std::memcpy(ibData, indices, ibSize);
        ib->setBuffer(*m_Engine, filament::IndexBuffer::BufferDescriptor(ibData, ibSize, [](void* b, size_t, void*) { delete[] static_cast<uint32_t*>(b); }));

        float minX = 1e30f, minY = 1e30f, minZ = 1e30f;
        float maxX = -1e30f, maxY = -1e30f, maxZ = -1e30f;
        for (size_t i = 0; i < vertexCount; ++i) {
            float x = vertices[i * 8 + 0], y = vertices[i * 8 + 1], z = vertices[i * 8 + 2];
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
            if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
        }
        filament::math::float3 center{(minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f};
        filament::math::float3 halfExtent{(maxX - minX) / 2.0f, (maxY - minY) / 2.0f, (maxZ - minZ) / 2.0f};

        filament::Box box{center, halfExtent};

        filament::gltfio::MaterialKey materialConfig{};
        materialConfig.doubleSided = true;
        materialConfig.unlit = false;
        materialConfig.hasBaseColorTexture = false;

        filament::gltfio::UvMap uvmap{};
        uvmap[0] = filament::gltfio::UvSet::UV0;

        filament::MaterialInstance* materialInstance = m_MaterialProvider->createMaterialInstance(&materialConfig, &uvmap, "ProceduralMaterial");

        utils::Entity customEntity = utils::EntityManager::get().create();
        auto& tcm = m_Engine->getTransformManager();
        tcm.create(customEntity);

        filament::RenderableManager::Builder(1)
                .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, vb, ib)
                .material(0, materialInstance)
                .boundingBox(box)
                .build(*m_Engine, customEntity);

        MeshHandle h = m_NextHandle++;
        Entry entry;
        entry.vertexBuffer = vb;
        entry.indexBuffer = ib;
        entry.customEntity = customEntity;
        entry.path = "procedural_" + std::to_string(h);

        m_Meshes[h] = entry;
        return h;
    }

    void ResourceManagerImpl::unloadMesh(MeshHandle handle) {
        auto it = m_Meshes.find(handle);
        if (it == m_Meshes.end()) return;
        m_PathToHandle.erase(it->second.path);

        if (it->second.asset) {
            m_AssetLoader->destroyAsset(it->second.asset);
        } else {
            if (it->second.vertexBuffer) m_Engine->destroy(it->second.vertexBuffer);
            if (it->second.indexBuffer)  m_Engine->destroy(it->second.indexBuffer);
            if (it->second.customEntity) {
                m_Engine->destroy(it->second.customEntity);
                utils::EntityManager::get().destroy(it->second.customEntity);
            }
        }
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

        if (it->second.asset) {
            return it->second.asset->getRoot();
        } else {
            return it->second.customEntity;
        }
    }

    void ResourceManagerImpl::addAssetToScene(MeshHandle h, filament::Scene* scene) {
        if (!g_Instance || !scene) return;
        auto it = g_Instance->m_Meshes.find(h);
        if (it == g_Instance->m_Meshes.end()) return;

        if (it->second.asset) {
            scene->addEntities(it->second.asset->getEntities(), it->second.asset->getEntityCount());
        } else if (it->second.customEntity) {
            scene->addEntity(it->second.customEntity);
        }
    }

    filament::gltfio::FilamentAsset* ResourceManagerImpl::getAsset(MeshHandle h) {
        if (!g_Instance) return nullptr;
        auto it = g_Instance->m_Meshes.find(h);
        if (it == g_Instance->m_Meshes.end()) return nullptr;
        return it->second.asset;
    }

    ResourceManagerImpl::Entry* ResourceManagerImpl::getEntry(MeshHandle h) {
        if (!g_Instance) return nullptr;
        auto it = g_Instance->m_Meshes.find(h);
        if (it == g_Instance->m_Meshes.end()) return nullptr;
        return &it->second;
    }

    utils::Entity ResourceManagerImpl::createInstance(MeshHandle h, filament::Scene* scene) {
        if (!g_Instance) return {};
        auto it = g_Instance->m_Meshes.find(h);
        if (it == g_Instance->m_Meshes.end()) return {};

        if (it->second.asset) {
            auto* inst = g_Instance->m_AssetLoader->createInstance(it->second.asset);
            if (inst && scene) {
                scene->addEntities(inst->getEntities(), inst->getEntityCount());
                return inst->getRoot();
            }
        } else if (it->second.vertexBuffer && it->second.indexBuffer) {
            if (scene && it->second.customEntity) {
                scene->addEntity(it->second.customEntity);
            }
            return it->second.customEntity;
        }
        return {};
    }
}
