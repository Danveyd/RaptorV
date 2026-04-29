#include "RaptorEngine.h"
#include "../scene/Entity.h"
#include "../scene/Components.h"
#include <gltfio/materials/uberarchive.h>
#include <gltfio/TextureProvider.h>

void RaptorEngine::init(AAssetManager* assetManager) {
    m_AssetManager = assetManager;

    m_Renderer = std::make_unique<raptor::Renderer>();
    m_ActiveScene = std::make_unique<raptor::Scene>(m_Renderer->getEngine());

    m_NameManager = new utils::NameComponentManager(utils::EntityManager::get());
    m_MaterialProvider = filament::gltfio::createUbershaderProvider(
            m_Renderer->getEngine(), UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

    filament::gltfio::AssetConfiguration assetConfig = {
            m_Renderer->getEngine(), m_MaterialProvider, m_NameManager, &utils::EntityManager::get()};
    m_AssetLoader = filament::gltfio::AssetLoader::create(assetConfig);
}

void RaptorEngine::onSurfaceCreated(ANativeWindow* window) {
    m_Renderer->onSurfaceCreated(window);
}

void RaptorEngine::onSurfaceChanged(int width, int height) {
    m_Renderer->onSurfaceChanged(width, height);
}

void RaptorEngine::onSurfaceDestroyed() {
    m_Renderer->onSurfaceDestroyed();
}

void RaptorEngine::update(float deltaTime) {
    if (!m_Renderer || !m_ActiveScene) return;

    m_ActiveScene->update(deltaTime);

    m_Renderer->beginFrame();
    m_Renderer->render();
    m_Renderer->endFrame();
}

uint32_t RaptorEngine::createEntity(const std::string& name) {
    raptor::Entity entity = m_ActiveScene->createEntity(name);
    return static_cast<uint32_t>(entity.getHandle());
}

void RaptorEngine::destroyEntity(uint32_t entityId) {
    entt::entity e = static_cast<entt::entity>(entityId);
    m_ActiveScene->destroyEntity(raptor::Entity(e, m_ActiveScene.get()));
}

void RaptorEngine::setPosition(uint32_t entityId, float x, float y, float z) {
    entt::entity e = static_cast<entt::entity>(entityId);
    raptor::Entity entity(e, m_ActiveScene.get());
    if (entity.hasComponent<raptor::TransformComponent>()) {
        entity.getComponent<raptor::TransformComponent>().position = {x, y, z};
    }
}

void RaptorEngine::setScale(uint32_t entityId, float x, float y, float z) {
    entt::entity e = static_cast<entt::entity>(entityId);
    raptor::Entity entity(e, m_ActiveScene.get());
    if (entity.hasComponent<raptor::TransformComponent>()) {
        entity.getComponent<raptor::TransformComponent>().scale = {x, y, z};
    }
}

void RaptorEngine::loadGlTF(uint32_t entityId, const std::string& path) {
    AAsset* asset = AAssetManager_open(m_AssetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) return;

    size_t size = AAsset_getLength(asset);
    const uint8_t* data = (const uint8_t*)AAsset_getBuffer(asset);

    filament::gltfio::FilamentAsset* filAsset = m_AssetLoader->createAsset(data, size);
    AAsset_close(asset);

    if (!filAsset) return;

    filament::gltfio::ResourceConfiguration resConfig = {};
    resConfig.engine = m_Renderer->getEngine();
    resConfig.normalizeSkinningWeights = true;
    m_ResourceLoader = new filament::gltfio::ResourceLoader(resConfig);

    auto stbProvider = filament::gltfio::createStbProvider(m_Renderer->getEngine());
    m_ResourceLoader->addTextureProvider("image/png", stbProvider);
    m_ResourceLoader->addTextureProvider("image/jpeg", stbProvider);

    m_ResourceLoader->loadResources(filAsset);
    filAsset->releaseSourceData();

    m_Renderer->getScene()->addEntities(filAsset->getEntities(), filAsset->getEntityCount());
    m_Assets.push_back(filAsset);

    entt::entity e = static_cast<entt::entity>(entityId);
    raptor::Entity entity(e, m_ActiveScene.get());
    entity.addComponent<raptor::MeshComponent>(filAsset->getRoot());
}

void RaptorEngine::setRotation(uint32_t entityId, float x, float y, float z) {
    raptor::Entity entity(static_cast<entt::entity>(entityId), m_ActiveScene.get());
    if (entity.hasComponent<raptor::TransformComponent>()) {
        entity.getComponent<raptor::TransformComponent>().rotation = {x, y, z};
    }
}

filament::math::float3 RaptorEngine::getPosition(uint32_t entityId) {
    raptor::Entity entity(static_cast<entt::entity>(entityId), m_ActiveScene.get());
    if (entity.hasComponent<raptor::TransformComponent>()) {
        return entity.getComponent<raptor::TransformComponent>().position;
    }
    return {0,0,0};
}

filament::math::float3 RaptorEngine::getRotation(uint32_t entityId) {
    raptor::Entity entity(static_cast<entt::entity>(entityId), m_ActiveScene.get());
    if (entity.hasComponent<raptor::TransformComponent>()) {
        return entity.getComponent<raptor::TransformComponent>().rotation;
    }
    return {0,0,0};
}

void RaptorEngine::addBoxCollider(uint32_t entityId, float hx, float hy, float hz, float mass) {
    raptor::Entity entity(static_cast<entt::entity>(entityId), m_ActiveScene.get());

    filament::math::float3 pos = {0,0,0};
    if (entity.hasComponent<raptor::TransformComponent>()) {
        pos = entity.getComponent<raptor::TransformComponent>().position;
    }

    uint32_t joltBodyId = m_ActiveScene->getPhysicsWorld()->createBoxBody(pos, {hx, hy, hz}, mass);

    entity.addComponent<raptor::RigidBodyComponent>(joltBodyId);
}
