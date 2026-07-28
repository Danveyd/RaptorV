#include <jni.h>
#include "raptor/Engine.h"
#include "../src/platform/android/AndroidFileSystem.h"
#include "../src/platform/android/AndroidWindow.h"
#include "scene/SceneImpl.h"
#include "resources/ResourceManagerImpl.h"
#include <filament/RenderableManager.h>
#include <filament/MaterialInstance.h>
#include <filament/Skybox.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <memory>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <android/bitmap.h>
#include <filament/TextureSampler.h>
#include "../src/render/RendererImpl.h"
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/RenderTarget.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/ColorGrading.h>
#include <filament/ToneMapper.h>
#include <functional>
#include <algorithm>
#include <gltfio/Animator.h>
#include <filament/InstanceBuffer.h>
#include <filamat/MaterialBuilder.h>
#include <android/log.h>
#include <math.h>
#include <vector>
#include <cstring>
#include <filament/LightManager.h>
#include <filament/SkinningBuffer.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CHECK_ENGINE(handle) \
    auto* wrapper = reinterpret_cast<EngineWrapper*>(handle); \
    if (!wrapper || reinterpret_cast<uintptr_t>(wrapper) < 0x10000 || !wrapper->engine) return;

#define CHECK_SCENE(handle) \
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(handle); \
    if (!scenePtr || reinterpret_cast<uintptr_t>(scenePtr) < 0x10000 || !*scenePtr) return;

using namespace raptor;

struct EngineWrapper {
    std::unique_ptr<IFileSystem> fs;
    std::unique_ptr<Engine> engine;
};

namespace filament {
    struct VirtualMachineEnv {
        static void JNI_OnLoad(JavaVM* vm);
    };
}

static void applyToRenderableInstances(
        SceneImpl* sceneImpl, EntityId entityId,
        std::function<void(filament::RenderableManager&, filament::RenderableManager::Instance)> func) {

    entt::entity e = unpackEntity(entityId);
    auto& reg = sceneImpl->registry();
    if (!reg.valid(e)) return;

    auto* mc = reg.try_get<components::MeshC>(e);
    if (!mc || mc->meshHandle == INVALID_MESH) return;

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    if (!fEngine) return;

    auto& rcm = fEngine->getRenderableManager();
    auto* asset = ResourceManagerImpl::getAsset(mc->meshHandle);

    if (asset) {
        const utils::Entity* renderableEntities = asset->getRenderableEntities();
        size_t count = asset->getRenderableEntityCount();
        for (size_t i = 0; i < count; ++i) {
            auto instance = rcm.getInstance(renderableEntities[i]);
            if (instance) func(rcm, instance);
        }
    } else if (mc->rootEntity) {
        auto instance = rcm.getInstance(mc->rootEntity);
        if (instance) func(rcm, instance);
    }
}

static void applyToMaterialInstances(SceneImpl* sceneImpl, EntityId entityId, std::function<void(filament::MaterialInstance*)> func) {
    entt::entity e = unpackEntity(entityId);
    auto& reg = sceneImpl->registry();
    if (!reg.valid(e)) return;

    auto* mc = reg.try_get<components::MeshC>(e);
    if (!mc) return;

    for (auto* mi : mc->materialInstances) {
        if (mi) func(mi);
    }
}

static void rgbeToFloat(const uint8_t* rgbe, float* rgb) {
    if (rgbe[3] != 0) {
        float f = ldexpf(1.0f, static_cast<int>(rgbe[3]) - (128 + 8));
        rgb[0] = rgbe[0] * f;
        rgb[1] = rgbe[1] * f;
        rgb[2] = rgbe[2] * f;
    } else {
        rgb[0] = rgb[1] = rgb[2] = 0.0f;
    }
}

static bool parseHDRBuffer(const uint8_t* buffer, size_t size, int& width, int& height, std::vector<float>& floatPixels) {
    const char* resLine = strstr(reinterpret_cast<const char*>(buffer), "-Y ");
    if (!resLine) resLine = strstr(reinterpret_cast<const char*>(buffer), "+Y ");
    if (!resLine) return false;

    sscanf(resLine, "-Y %d +X %d", &height, &width);
    if (width <= 0 || height <= 0) return false;

    const uint8_t* data = reinterpret_cast<const uint8_t*>(strchr(resLine, '\n') + 1);
    const uint8_t* end = buffer + size;

    floatPixels.resize(width * height * 3);

    for (int y = 0; y < height; ++y) {
        if (data >= end) break;

        if (width >= 8 && width <= 0x7fff && data[0] == 2 && data[1] == 2 && data[2] == ((width >> 8) & 0xff) && data[3] == (width & 0xff)) {
            data += 4;
            std::vector<uint8_t> scanline(4 * width);
            for (int channel = 0; channel < 4; ++channel) {
                for (int x = 0; x < width; ) {
                    uint8_t code = *data++;
                    if (code > 128) {
                        int count = code - 128;
                        uint8_t val = *data++;
                        while (count-- > 0 && x < width) scanline[channel * width + (x++)] = val;
                    } else {
                        int count = code;
                        while (count-- > 0 && x < width) scanline[channel * width + (x++)] = *data++;
                    }
                }
            }
            for (int x = 0; x < width; ++x) {
                uint8_t rgbe[4] = { scanline[0 * width + x], scanline[1 * width + x], scanline[2 * width + x], scanline[3 * width + x] };
                rgbeToFloat(rgbe, &floatPixels[(y * width + x) * 3]);
            }
        } else {
            for (int x = 0; x < width; ++x) {
                rgbeToFloat(data, &floatPixels[(y * width + x) * 3]);
                data += 4;
            }
        }
    }
    return true;
}

static filament::LinearToneMapper s_LinearMapper;
static filament::ACESToneMapper s_AcesMapper;
static filament::ACESLegacyToneMapper s_AcesLegacyMapper;
static filament::FilmicToneMapper s_FilmicMapper;
static filament::AgxToneMapper s_AgxMapper;
static filament::PBRNeutralToneMapper s_PbrNeutralMapper;
static filament::DisplayRangeToneMapper s_DisplayRangeMapper;

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    filament::VirtualMachineEnv::JNI_OnLoad(vm);

    return JNI_VERSION_1_6;
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createEngine(JNIEnv* env, jclass, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);

    auto* wrapper = new EngineWrapper();
    wrapper->fs = std::make_unique<AndroidFileSystem>(mgr);

    EngineConfig cfg;
    cfg.fileSystem = wrapper->fs.get();
    wrapper->engine = Engine::create(cfg);

    return reinterpret_cast<jlong>(wrapper);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroyEngine(JNIEnv* env, jclass, jlong engineHandle) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    delete wrapper;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_onSurfaceCreated(JNIEnv* env, jclass, jlong engineHandle, jobject surface) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    if (!wrapper || !wrapper->engine) return;

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) return;

    int width = ANativeWindow_getWidth(window);
    int height = ANativeWindow_getHeight(window);

    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    AndroidWindow aw(window, width, height);
    wrapper->engine->onSurfaceCreated(&aw);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_onSurfaceChanged(JNIEnv* env, jclass, jlong engineHandle, jint width, jint height) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    wrapper->engine->onSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_onSurfaceDestroyed(JNIEnv* env, jclass, jlong engineHandle) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    wrapper->engine->onSurfaceDestroyed();
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_tick(JNIEnv* env, jclass, jlong engineHandle, jfloat dt) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    wrapper->engine->tick(dt);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createScene(JNIEnv* env, jclass, jlong engineHandle) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto scene = wrapper->engine->createScene();

    auto* scenePtr = new std::shared_ptr<Scene>(scene);
    return reinterpret_cast<jlong>(scenePtr);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setActiveScene(JNIEnv* env, jclass, jlong engineHandle, jlong sceneHandle) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    wrapper->engine->setActiveScene(*scenePtr);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroyScene(JNIEnv* env, jclass, jlong sceneHandle) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    delete scenePtr;
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createEntity(JNIEnv* env, jclass, jlong sceneHandle, jstring name) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    const char* cName = env->GetStringUTFChars(name, nullptr);
    EntityId id = (*scenePtr)->createEntity(cName);
    env->ReleaseStringUTFChars(name, cName);
    return static_cast<jlong>(id);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setTransform(JNIEnv* env, jclass, jlong sceneHandle, jlong entityId,
                                                               jfloat px, jfloat py, jfloat pz, jfloat rx, jfloat ry, jfloat rz, jfloat sx, jfloat sy, jfloat sz) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    Transform t;
    t.position = {px, py, pz};
    t.rotation = {rx, ry, rz};
    t.scale    = {sx, sy, sz};
    (*scenePtr)->setTransform(static_cast<EntityId>(entityId), t);
}

JNIEXPORT jfloatArray JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_getTransformRotation(JNIEnv* env, jclass, jlong sceneHandle, jlong entityId) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    Transform t = (*scenePtr)->getTransform(static_cast<EntityId>(entityId));

    jfloatArray result = env->NewFloatArray(3);
    float arr[3] = {t.rotation.x, t.rotation.y, t.rotation.z};
    env->SetFloatArrayRegion(result, 0, 3, arr);
    return result;
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_loadMesh(JNIEnv* env, jclass, jlong engineHandle, jstring path) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    const char* cPath = env->GetStringUTFChars(path, nullptr);
    MeshHandle h = wrapper->engine->getResources()->loadMesh(cPath);
    env->ReleaseStringUTFChars(path, cPath);
    return static_cast<jlong>(h);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_attachMesh(JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jlong meshHandle) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    (*scenePtr)->attachMesh(static_cast<EntityId>(entityId), static_cast<MeshHandle>(meshHandle));
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_attachRigidBody(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId,
        jint type, jfloat arg1, jfloat arg2, jfloat arg3, jfloat mass) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    RigidBodyDesc desc;
    desc.type = static_cast<ColliderType>(type);
    desc.mass = mass;

    if (desc.type == ColliderType::Box) {
        desc.halfExtent = {arg1, arg2, arg3};
    } else if (desc.type == ColliderType::Sphere) {
        desc.radius = arg1;
    } else if (desc.type == ColliderType::Capsule) {
        desc.halfHeight = arg1;
        desc.radius = arg2;
    }

    sceneImpl->attachRigidBody(static_cast<EntityId>(entityId), desc);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMainCamera(JNIEnv* env, jclass, jlong engineHandle, jlong cameraId) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    wrapper->engine->getRenderer()->setMainCamera(static_cast<EntityId>(cameraId));
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createCamera(JNIEnv* env,
                                                               jclass, jlong engineHandle,
                                                               jint projectionType, jfloat fov, jfloat focalLength, jfloat near, jfloat far, jfloat focusDist,
                                                               jfloat px, jfloat py, jfloat pz, jfloat tx, jfloat ty, jfloat tz, jfloat ux, jfloat uy, jfloat uz,
                                                               jfloat aperture, jfloat shutterSpeed, jfloat sensitivityISO,
                                                               jfloat shiftX, jfloat shiftY,
                                                               jfloat orthoLeft, jfloat orthoRight, jfloat orthoBottom, jfloat orthoTop) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    CameraDesc desc;
    desc.projectionType = static_cast<ProjectionType>(projectionType);
    desc.fovDegrees = fov;
    desc.focalLength = focalLength;
    desc.nearPlane = near;
    desc.farPlane = far;
    desc.focusDistance = focusDist;

    desc.position = {px, py, pz};
    desc.target = {tx, ty, tz};
    desc.up = {ux, uy, uz};

    desc.aperture = aperture;
    desc.shutterSpeed = shutterSpeed;
    desc.sensitivityISO = sensitivityISO;

    desc.shiftX = shiftX;
    desc.shiftY = shiftY;

    desc.orthoLeft = orthoLeft;
    desc.orthoRight = orthoRight;
    desc.orthoBottom = orthoBottom;
    desc.orthoTop = orthoTop;

    EntityId id = wrapper->engine->getRenderer()->createCamera(desc);
    return static_cast<jlong>(id);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_updateCamera(JNIEnv* env,
                                                               jclass, jlong engineHandle, jlong cameraId,
                                                               jint projectionType, jfloat fov, jfloat focalLength, jfloat near, jfloat far, jfloat focusDist,
                                                               jfloat px, jfloat py, jfloat pz, jfloat tx, jfloat ty, jfloat tz, jfloat ux, jfloat uy, jfloat uz,
                                                               jfloat aperture, jfloat shutterSpeed, jfloat sensitivityISO,
                                                               jfloat shiftX, jfloat shiftY,
                                                               jfloat orthoLeft, jfloat orthoRight, jfloat orthoBottom, jfloat orthoTop) {

    auto *wrapper = reinterpret_cast<EngineWrapper *>(engineHandle);
    CameraDesc desc;
    desc.projectionType = static_cast<ProjectionType>(projectionType);
    desc.fovDegrees = fov;
    desc.focalLength = focalLength;
    desc.nearPlane = near;
    desc.farPlane = far;
    desc.focusDistance = focusDist;

    desc.position = {px, py, pz};
    desc.target = {tx, ty, tz};
    desc.up = {ux, uy, uz};

    desc.aperture = aperture;
    desc.shutterSpeed = shutterSpeed;
    desc.sensitivityISO = sensitivityISO;

    desc.shiftX = shiftX;
    desc.shiftY = shiftY;

    desc.orthoLeft = orthoLeft;
    desc.orthoRight = orthoRight;
    desc.orthoBottom = orthoBottom;
    desc.orthoTop = orthoTop;

    wrapper->engine->getRenderer()->updateCamera(static_cast<EntityId>(cameraId), desc);
}


JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setAmbientOcclusionBasic(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jint aoType, jfloat radius, jfloat power, jfloat bias,
        jfloat intensity, jint quality, jboolean bentNormals, jfloat resolution) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::AmbientOcclusionOptions ao = rendererImpl->getView()->getAmbientOcclusionOptions();
    ao.enabled = enabled;
    ao.aoType = static_cast<filament::AmbientOcclusionOptions::AmbientOcclusionType>(aoType);
    ao.radius = radius;
    ao.power = power;
    ao.bias = bias;
    ao.intensity = intensity;
    ao.quality = static_cast<filament::QualityLevel>(quality);
    ao.bentNormals = bentNormals;
    ao.resolution = resolution;

    rendererImpl->getView()->setAmbientOcclusionOptions(ao);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setSSCTOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat coneRad, jfloat shadowDist, jfloat contactDistMax,
        jfloat intensity, jfloat dirX, jfloat dirY, jfloat dirZ) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::AmbientOcclusionOptions ao = rendererImpl->getView()->getAmbientOcclusionOptions();
    ao.ssct.enabled = enabled;
    ao.ssct.lightConeRad = coneRad;
    ao.ssct.shadowDistance = shadowDist;
    ao.ssct.contactDistanceMax = contactDistMax;
    ao.ssct.intensity = intensity;
    ao.ssct.lightDirection = {dirX, dirY, dirZ};

    rendererImpl->getView()->setAmbientOcclusionOptions(ao);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setGTAOOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jint sliceCount, jint stepsPerSlice, jfloat thicknessHeuristic) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::AmbientOcclusionOptions ao = rendererImpl->getView()->getAmbientOcclusionOptions();
    ao.gtao.sampleSliceCount = sliceCount;
    ao.gtao.sampleStepsPerSlice = stepsPerSlice;
    ao.gtao.thicknessHeuristic = thicknessHeuristic;

    rendererImpl->getView()->setAmbientOcclusionOptions(ao);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setScreenSpaceRefractionEnabled(
        JNIEnv*, jclass, jlong engineHandle, jboolean enabled) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    rendererImpl->getView()->setScreenSpaceRefractionEnabled(enabled);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setDitheringEnabled(
        JNIEnv*, jclass, jlong engineHandle, jboolean enabled) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    rendererImpl->getView()->setDithering(enabled ? filament::View::Dithering::TEMPORAL
                                                  : filament::View::Dithering::NONE);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setTemporalAntiAliasingOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat feedback, jfloat lodBias, jfloat sharpness, jfloat upscaling,
        jboolean filterHistory, jboolean filterInput, jboolean useYCoCg, jboolean hdr,
        jint boxType, jint boxClipping, jint jitterPattern, jfloat varianceGamma,
        jboolean preventFlickering, jboolean historyReprojection) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::TemporalAntiAliasingOptions taa;
    taa.enabled = enabled;
    taa.feedback = feedback;
    taa.lodBias = lodBias;
    taa.sharpness = sharpness;
    taa.upscaling = upscaling;
    taa.filterHistory = filterHistory;
    taa.filterInput = filterInput;
    taa.useYCoCg = useYCoCg;
    taa.hdr = hdr;
    taa.boxType = static_cast<filament::TemporalAntiAliasingOptions::BoxType>(boxType);
    taa.boxClipping = static_cast<filament::TemporalAntiAliasingOptions::BoxClipping>(boxClipping);
    taa.jitterPattern = static_cast<filament::TemporalAntiAliasingOptions::JitterPattern>(jitterPattern);
    taa.varianceGamma = varianceGamma;
    taa.preventFlickering = preventFlickering;
    taa.historyReprojection = historyReprojection;

    rendererImpl->getView()->setTemporalAntiAliasingOptions(taa);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setFogOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat distance, jfloat cutOffDistance, jfloat maximumOpacity,
        jfloat height, jfloat heightFalloff, jfloat colorR, jfloat colorG, jfloat colorB,
        jfloat density, jfloat inScatteringStart, jfloat inScatteringSize, jboolean fogColorFromIbl) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::FogOptions fog;
    fog.enabled = enabled;
    fog.distance = distance;
    fog.cutOffDistance = cutOffDistance;
    fog.maximumOpacity = maximumOpacity;
    fog.height = height;
    fog.heightFalloff = heightFalloff;
    fog.color = {colorR, colorG, colorB};
    fog.density = density;
    fog.inScatteringStart = inScatteringStart;
    fog.inScatteringSize = inScatteringSize;
    fog.fogColorFromIbl = fogColorFromIbl;

    rendererImpl->getView()->setFogOptions(fog);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setDepthOfFieldOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat cocScale, jfloat cocAspectRatio, jfloat maxApertureDiameter,
        jint filter, jboolean nativeResolution, jint fgRings, jint bgRings, jint fastGatherRings,
        jint maxFgCoc, jint maxBgCoc) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::DepthOfFieldOptions dof;
    dof.enabled = enabled;
    dof.cocScale = cocScale;
    dof.cocAspectRatio = cocAspectRatio;
    dof.maxApertureDiameter = maxApertureDiameter;
    dof.filter = static_cast<filament::DepthOfFieldOptions::Filter>(filter);
    dof.nativeResolution = nativeResolution;
    dof.foregroundRingCount = fgRings;
    dof.backgroundRingCount = bgRings;
    dof.fastGatherRingCount = fastGatherRings;
    dof.maxForegroundCOC = maxFgCoc;
    dof.maxBackgroundCOC = maxBgCoc;

    rendererImpl->getView()->setDepthOfFieldOptions(dof);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setVignetteOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat midPoint, jfloat roundness, jfloat feather,
        jfloat colorR, jfloat colorG, jfloat colorB, jfloat colorA) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::VignetteOptions vignette;
    vignette.enabled = enabled;
    vignette.midPoint = midPoint;
    vignette.roundness = roundness;
    vignette.feather = feather;
    vignette.color = {colorR, colorG, colorB, colorA};

    rendererImpl->getView()->setVignetteOptions(vignette);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setDynamicResolutionOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jboolean homogeneousScaling, jfloat minScaleX, jfloat minScaleY,
        jfloat maxScaleX, jfloat maxScaleY, jfloat sharpness, jint quality) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::DynamicResolutionOptions dyn;
    dyn.enabled = enabled;
    dyn.homogeneousScaling = homogeneousScaling;
    dyn.minScale = {minScaleX, minScaleY};
    dyn.maxScale = {maxScaleX, maxScaleY};
    dyn.sharpness = sharpness;
    dyn.quality = static_cast<filament::QualityLevel>(quality);

    rendererImpl->getView()->setDynamicResolutionOptions(dyn);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setScreenSpaceReflectionsOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat thickness, jfloat bias, jfloat maxDistance, jfloat stride) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->getView()) return;

    filament::ScreenSpaceReflectionsOptions ssr = rendererImpl->getView()->getScreenSpaceReflectionsOptions();
    ssr.enabled = enabled;
    ssr.thickness = thickness;
    ssr.bias = bias;
    ssr.maxDistance = maxDistance;
    ssr.stride = stride;

    rendererImpl->getView()->setScreenSpaceReflectionsOptions(ssr);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setBloomOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jboolean enabled, jfloat strength, jint resolution, jint levels, jint blendMode,
        jboolean threshold, jfloat highlight, jint quality, jboolean lensFlare, jboolean starburst,
        jfloat chromaticAberration, jint ghostCount, jfloat ghostSpacing, jfloat ghostThreshold,
        jfloat haloThickness, jfloat haloRadius, jfloat haloThreshold, jfloat dirtStrength) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->getView()) return;

    filament::BloomOptions bloom = rendererImpl->getView()->getBloomOptions();
    bloom.enabled = enabled;
    bloom.strength = strength;
    bloom.resolution = resolution;
    bloom.levels = levels;
    bloom.blendMode = static_cast<filament::BloomOptions::BlendMode>(blendMode);
    bloom.threshold = threshold;
    bloom.highlight = highlight;
    bloom.quality = static_cast<filament::QualityLevel>(quality);
    bloom.lensFlare = lensFlare;
    bloom.starburst = starburst;

    rendererImpl->getView()->setBloomOptions(bloom);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setSoftShadowOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jfloat penumbraScale, jfloat penumbraRatioScale) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::SoftShadowOptions soft;
    soft.penumbraScale = penumbraScale;
    soft.penumbraRatioScale = penumbraRatioScale;

    rendererImpl->getView()->setSoftShadowOptions(soft);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setColorGradingOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jint toneMapping, jfloat exposure, jfloat whiteBalanceTemp, jfloat whiteBalanceTint,
        jfloat contrast, jfloat vibrance, jfloat saturation,
        jfloat shadowsR, jfloat shadowsG, jfloat shadowsB, jfloat shadowsW,
        jfloat midtonesR, jfloat midtonesG, jfloat midtonesB, jfloat midtonesW,
        jfloat highlightsR, jfloat highlightsG, jfloat highlightsB, jfloat highlightsW,
        jfloat rangeX, jfloat rangeY, jfloat rangeZ, jfloat rangeW,
        jfloat slopeX, jfloat slopeY, jfloat slopeZ,
        jfloat offsetX, jfloat offsetY, jfloat offsetZ,
        jfloat powerX, jfloat powerY, jfloat powerZ) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();
    filament::View* view = rendererImpl->getView();

    const filament::ToneMapper* mapper = &s_AcesMapper;

    switch (toneMapping) {
        case 0: mapper = &s_LinearMapper; break;
        case 1: mapper = &s_AcesMapper; break;
        case 2: mapper = &s_AcesLegacyMapper; break;
        case 3: mapper = &s_FilmicMapper; break;
        case 4: mapper = &s_AgxMapper; break;
        case 5: mapper = &s_PbrNeutralMapper; break;
        case 6: mapper = &s_DisplayRangeMapper; break;
    }

    filament::ColorGrading* colorGrading = filament::ColorGrading::Builder()
            .toneMapper(mapper)
            .exposure(exposure)
            .whiteBalance(whiteBalanceTemp, whiteBalanceTint)
            .contrast(contrast)
            .vibrance(vibrance)
            .saturation(saturation)
            .shadowsMidtonesHighlights(
                    {shadowsR, shadowsG, shadowsB, shadowsW},
                    {midtonesR, midtonesG, midtonesB, midtonesW},
                    {highlightsR, highlightsG, highlightsB, highlightsW},
                    {rangeX, rangeY, rangeZ, rangeW}
            )
            .slopeOffsetPower(
                    {slopeX, slopeY, slopeZ},
                    {offsetX, offsetY, offsetZ},
                    {powerX, powerY, powerZ}
            )
            .build(*fEngine);

    auto* oldColorGrading = const_cast<filament::ColorGrading*>(view->getColorGrading());
    view->setColorGrading(colorGrading);
    if (oldColorGrading) {
        fEngine->destroy(oldColorGrading);
    }
}


JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setIndirectLightIntensity(
        JNIEnv*, jclass, jlong sceneHandle, jfloat intensity) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    (*scenePtr)->setIndirectLightIntensity(intensity);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createLight(JNIEnv* env,
                                                              jclass, jlong engineHandle, jint type, jfloat cx, jfloat cy, jfloat cz, jfloat intensity,
                                                              jfloat dx, jfloat dy, jfloat dz, jfloat px, jfloat py, jfloat pz, jfloat falloff, jboolean shadows, jboolean enableContactShadows,
                                                              jfloat innerCone, jfloat outerCone,
                                                              jint shadowMapSize, jint shadowCascades, jfloat shadowConstantBias, jfloat shadowNormalBias,
                                                              jfloat sunAngularRadius, jfloat sunHaloSize, jfloat sunHaloFalloff,
                                                              jfloat split1, jfloat split2, jfloat split3) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    LightDesc desc;
    desc.type = static_cast<LightType>(type);
    desc.color = {cx, cy, cz};
    desc.intensity = intensity;
    desc.direction = {dx, dy, dz};
    desc.position = {px, py, pz};
    desc.falloffRadius = falloff;
    desc.castShadows = shadows;
    desc.enableContactShadows = enableContactShadows;
    desc.innerConeAngle = innerCone;
    desc.outerConeAngle = outerCone;

    desc.shadowMapSize = shadowMapSize;
    desc.shadowCascades = shadowCascades;
    desc.shadowConstantBias = shadowConstantBias;
    desc.shadowNormalBias = shadowNormalBias;

    desc.sunAngularRadius = sunAngularRadius;
    desc.sunHaloSize = sunHaloSize;
    desc.sunHaloFalloff = sunHaloFalloff;
    desc.cascadeSplit1 = split1;
    desc.cascadeSplit2 = split2;
    desc.cascadeSplit3 = split3;

    EntityId id = wrapper->engine->getRenderer()->createLight(desc);
    return static_cast<jlong>(id);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_updateLight(JNIEnv* env,
                                                              jclass, jlong engineHandle, jlong lightId, jint type, jfloat cx, jfloat cy,
                                                              jfloat cz, jfloat intensity, jfloat dx, jfloat dy, jfloat dz, jfloat px, jfloat
                                                              py, jfloat pz, jfloat falloff, jboolean shadows, jboolean enableContactShadows, jfloat innerCone, jfloat outerCone,
                                                              jint shadowMapSize, jint shadowCascades, jfloat shadowConstantBias, jfloat shadowNormalBias,
                                                              jfloat sunAngularRadius, jfloat sunHaloSize, jfloat sunHaloFalloff,
                                                              jfloat split1, jfloat split2, jfloat split3) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    LightDesc desc;
    desc.type = static_cast<LightType>(type);
    desc.color = {cx, cy, cz};
    desc.intensity = intensity;
    desc.direction = {dx, dy, dz};
    desc.position = {px, py, pz};
    desc.falloffRadius = falloff;
    desc.castShadows = shadows;
    desc.enableContactShadows = enableContactShadows;
    desc.innerConeAngle = innerCone;
    desc.outerConeAngle = outerCone;

    desc.shadowMapSize = shadowMapSize;
    desc.shadowCascades = shadowCascades;
    desc.shadowConstantBias = shadowConstantBias;
    desc.shadowNormalBias = shadowNormalBias;

    desc.sunAngularRadius = sunAngularRadius;
    desc.sunHaloSize = sunHaloSize;
    desc.sunHaloFalloff = sunHaloFalloff;
    desc.cascadeSplit1 = split1;
    desc.cascadeSplit2 = split2;
    desc.cascadeSplit3 = split3;

    wrapper->engine->getRenderer()->updateLight(static_cast<EntityId>(lightId), desc);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setIndirectLightRotation(
        JNIEnv*, jclass, jlong sceneHandle, jfloat rotationY) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    (*scenePtr)->setIndirectLightRotation(rotationY);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setTransformBuffer(
        JNIEnv* env, jclass, jlong sceneHandle, jobject byteBuffer) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    if (byteBuffer == nullptr) {
        sceneImpl->setTransformBuffer(nullptr, 0);
        return;
    }

    float* bufferPtr = reinterpret_cast<float*>(env->GetDirectBufferAddress(byteBuffer));
    jlong capacity = env->GetDirectBufferCapacity(byteBuffer);
    size_t floatCount = static_cast<size_t>(capacity / sizeof(float));

    sceneImpl->setTransformBuffer(bufferPtr, floatCount);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityParent(
        JNIEnv* env, jclass, jlong sceneHandle, jlong childId, jlong parentId) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    if (!fEngine) return;

    auto& tcm = fEngine->getTransformManager();
    auto& reg = sceneImpl->registry();

    entt::entity childEnt = unpackEntity(static_cast<EntityId>(childId));
    entt::entity parentEnt = unpackEntity(static_cast<EntityId>(parentId));

    if (!reg.valid(childEnt) || !reg.valid(parentEnt)) return;

    utils::Entity childFil;
    if (auto* mc = reg.try_get<components::MeshC>(childEnt)) childFil = mc->rootEntity;
    if (auto* cc = reg.try_get<components::CameraC>(childEnt)) childFil = cc->filamentEntity;
    if (auto* lc = reg.try_get<components::LightC>(childEnt)) childFil = lc->filamentEntity;

    utils::Entity parentFil;
    if (auto* mc = reg.try_get<components::MeshC>(parentEnt)) parentFil = mc->rootEntity;
    if (auto* cc = reg.try_get<components::CameraC>(parentEnt)) parentFil = cc->filamentEntity;
    if (auto* lc = reg.try_get<components::LightC>(parentEnt)) parentFil = lc->filamentEntity;

    if (childFil && parentFil) {
        auto childInst = tcm.getInstance(childFil);
        auto parentInst = tcm.getInstance(parentFil);

        if (!childInst.isValid()) { tcm.create(childFil); childInst = tcm.getInstance(childFil); }
        if (!parentInst.isValid()) { tcm.create(parentFil); parentInst = tcm.getInstance(parentFil); }

        if (childInst.isValid() && parentInst.isValid()) {
            tcm.setParent(childInst, parentInst);
        }
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setSkyboxColor(
        JNIEnv* env, jclass, jlong sceneHandle, jfloat r, jfloat g, jfloat b, jfloat a) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    filament::Scene* fScene = sceneImpl->filamentScene();
    if (!fEngine || !fScene) return;

    filament::Skybox* skybox = filament::Skybox::Builder()
            .color({r, g, b, a})
            .build(*fEngine);

    auto* oldSkybox = fScene->getSkybox();
    if (oldSkybox) {
        fEngine->destroy(oldSkybox);
    }

    fScene->setSkybox(skybox);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEnvironment(
        JNIEnv* env, jclass, jlong sceneHandle,
        jbyteArray iblBytes, jint iblSize,
        jbyteArray skyboxBytes, jint skyboxSize) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    jbyte* iblData = iblBytes ? env->GetByteArrayElements(iblBytes, nullptr) : nullptr;
    jbyte* skyboxData = skyboxBytes ? env->GetByteArrayElements(skyboxBytes, nullptr) : nullptr;

    sceneImpl->setEnvironment(
            reinterpret_cast<const uint8_t*>(iblData), static_cast<size_t>(iblSize),
            reinterpret_cast<const uint8_t*>(skyboxData), static_cast<size_t>(skyboxSize)
    );

    if (iblData) env->ReleaseByteArrayElements(iblBytes, iblData, JNI_ABORT);
    if (skyboxData) env->ReleaseByteArrayElements(skyboxBytes, skyboxData, JNI_ABORT);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createTextureFromBitmap(
        JNIEnv* env, jclass, jlong engineHandle, jobject bitmap) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    AndroidBitmapInfo info;
    void* pixels = nullptr;

    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return 0;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return 0;

    uint32_t width = info.width;
    uint32_t height = info.height;

    filament::Texture* texture = filament::Texture::Builder()
            .width(width)
            .height(height)
            .levels(1)
            .sampler(filament::Texture::Sampler::SAMPLER_2D)
            .format(filament::Texture::InternalFormat::RGBA8)
            .build(*fEngine);

    size_t size = width * height * 4;
    uint8_t* buffer = new uint8_t[size];
    std::memcpy(buffer, pixels, size);

    AndroidBitmap_unlockPixels(env, bitmap);

    filament::Texture::PixelBufferDescriptor desc(
            buffer, size,
            filament::Texture::Format::RGBA,
            filament::Texture::Type::UBYTE,
            [](void* buffer, size_t size, void* user) {
                delete[] static_cast<uint8_t*>(buffer);
            }
    );

    texture->setImage(*fEngine, 0, std::move(desc));

    return reinterpret_cast<jlong>(texture);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroyTexture(
        JNIEnv* env, jclass, jlong engineHandle, jlong textureHandle) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    auto* texture = reinterpret_cast<filament::Texture*>(textureHandle);
    if (texture) {
        fEngine->destroy(texture);
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialParamTexture(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jstring name, jlong textureHandle,
        jint minFilter, jint magFilter, jint wrapS, jint wrapT) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    auto* texture = reinterpret_cast<filament::Texture*>(textureHandle);
    if (!texture) return;

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    if (!fEngine) return;

    filament::TextureSampler sampler;
    sampler.setMinFilter(static_cast<filament::TextureSampler::MinFilter>(minFilter));
    sampler.setMagFilter(static_cast<filament::TextureSampler::MagFilter>(magFilter));
    sampler.setWrapModeS(static_cast<filament::TextureSampler::WrapMode>(wrapS));
    sampler.setWrapModeT(static_cast<filament::TextureSampler::WrapMode>(wrapT));

    const char* cName = env->GetStringUTFChars(name, nullptr);

    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [cName, texture, sampler](filament::MaterialInstance* mi) {
        mi->setParameter(cName, texture, sampler);

        if (std::strcmp(cName, "baseColorMap") == 0) mi->setParameter("baseColorIndex", 0);
        if (std::strcmp(cName, "normalMap") == 0)    mi->setParameter("normalIndex", 0);
        if (std::strcmp(cName, "metallicMap") == 0)  mi->setParameter("metallicIndex", 0);
        if (std::strcmp(cName, "roughnessMap") == 0) mi->setParameter("roughnessIndex", 0);
    });

    env->ReleaseStringUTFChars(name, cName);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createMesh(
        JNIEnv* env, jclass, jlong engineHandle, jfloatArray vertices, jintArray indices) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);

    jfloat* vData = env->GetFloatArrayElements(vertices, nullptr);
    jint* iData = env->GetIntArrayElements(indices, nullptr);

    jsize vSize = env->GetArrayLength(vertices) / 8;
    jsize iSize = env->GetArrayLength(indices);

    auto* resourceManagerImpl = static_cast<ResourceManagerImpl*>(wrapper->engine->getResources());
    MeshHandle h = resourceManagerImpl->createMesh(
            reinterpret_cast<const float*>(vData), static_cast<size_t>(vSize),
            reinterpret_cast<const uint32_t*>(iData), static_cast<size_t>(iSize)
    );

    env->ReleaseFloatArrayElements(vertices, vData, JNI_ABORT);
    env->ReleaseIntArrayElements(indices, iData, JNI_ABORT);

    return static_cast<jlong>(h);
}

JNIEXPORT jlongArray JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createRenderTarget(
        JNIEnv* env, jclass, jlong engineHandle, jint width, jint height) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    filament::Texture* colorTex = filament::Texture::Builder()
            .width(width).height(height).levels(1)
            .usage(filament::Texture::Usage::COLOR_ATTACHMENT | filament::Texture::Usage::SAMPLEABLE)
            .format(filament::Texture::InternalFormat::RGBA8)
            .build(*fEngine);

    filament::Texture* depthTex = filament::Texture::Builder()
            .width(width).height(height).levels(1)
            .usage(filament::Texture::Usage::DEPTH_ATTACHMENT)
            .format(filament::Texture::InternalFormat::DEPTH24)
            .build(*fEngine);

    filament::RenderTarget* rt = filament::RenderTarget::Builder()
            .texture(filament::RenderTarget::AttachmentPoint::COLOR, colorTex)
            .texture(filament::RenderTarget::AttachmentPoint::DEPTH, depthTex)
            .build(*fEngine);

    jlongArray result = env->NewLongArray(3);
    jlong handles[3] = {
            reinterpret_cast<jlong>(rt),
            reinterpret_cast<jlong>(colorTex),
            reinterpret_cast<jlong>(depthTex)
    };
    env->SetLongArrayRegion(result, 0, 3, handles);
    return result;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroyRenderTarget(
        JNIEnv* env, jclass, jlong engineHandle, jlong rtHandle, jlong colorTexHandle, jlong depthTexHandle) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    auto* rt = reinterpret_cast<filament::RenderTarget*>(rtHandle);
    auto* colorTex = reinterpret_cast<filament::Texture*>(colorTexHandle);
    auto* depthTex = reinterpret_cast<filament::Texture*>(depthTexHandle);

    if (rt) fEngine->destroy(rt);
    if (colorTex) fEngine->destroy(colorTex);
    if (depthTex) fEngine->destroy(depthTex);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createOffscreenView(
        JNIEnv* env, jclass, jlong engineHandle, jlong sceneHandle, jlong cameraId, jlong rtHandle, jint width, jint height) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    filament::View* offscreenView = fEngine->createView();

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return 0;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    offscreenView->setScene(sceneImpl->filamentScene());

    entt::entity camEnt = unpackEntity(static_cast<EntityId>(cameraId));
    auto* cc = sceneImpl->registry().try_get<components::CameraC>(camEnt);
    if (cc) {
        filament::Camera* fcam = fEngine->getCameraComponent(cc->filamentEntity);
        if (fcam) {
            offscreenView->setCamera(fcam);
        }
    }

    auto* rt = reinterpret_cast<filament::RenderTarget*>(rtHandle);
    offscreenView->setRenderTarget(rt);
    offscreenView->setViewport({0, 0, static_cast<uint32_t>(width), static_cast<uint32_t>(height)});

    offscreenView->setPostProcessingEnabled(false);

    offscreenView->setPostProcessingEnabled(false);

    rendererImpl->addOffscreenView(offscreenView);

    return reinterpret_cast<jlong>(offscreenView);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroyOffscreenView(
        JNIEnv* env, jclass, jlong engineHandle, jlong viewHandle) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    auto* view = reinterpret_cast<filament::View*>(viewHandle);
    if (view) {
        rendererImpl->removeOffscreenView(view);
        fEngine->destroy(view);
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityCastShadows(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean castShadows) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [castShadows](auto& rcm, auto inst) {
        rcm.setCastShadows(inst, castShadows);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityReceiveShadows(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean receiveShadows) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [receiveShadows](auto& rcm, auto inst) {
        rcm.setReceiveShadows(inst, receiveShadows);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityPriority(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jint priority) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    uint8_t p = static_cast<uint8_t>(std::clamp(priority, 0, 7));
    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [p](auto& rcm, auto inst) {
        rcm.setPriority(inst, p);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityCulling(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean culling) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [culling](auto& rcm, auto inst) {
        rcm.setCulling(inst, culling);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityLayerMask(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jint select, jint value) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    uint8_t sel = static_cast<uint8_t>(select);
    uint8_t val = static_cast<uint8_t>(value);
    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [sel, val](auto& rcm, auto inst) {
        rcm.setLayerMask(inst, sel, val);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialParamFloat(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jstring name, jfloat value) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    const char* cName = env->GetStringUTFChars(name, nullptr);
    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [cName, value](filament::MaterialInstance* mi) {
        mi->setParameter(cName, value);
    });
    env->ReleaseStringUTFChars(name, cName);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialParamVec2(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jstring name, jfloat x, jfloat y) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    const char* cName = env->GetStringUTFChars(name, nullptr);
    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [cName, x, y](filament::MaterialInstance* mi) {
        mi->setParameter(cName, filament::math::float2{x, y});
    });
    env->ReleaseStringUTFChars(name, cName);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialParamColor(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jstring name, jfloat r, jfloat g, jfloat b, jfloat a) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    const char* cName = env->GetStringUTFChars(name, nullptr);
    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [cName, r, g, b, a](filament::MaterialInstance* mi) {
        if (std::strcmp(cName, "baseColorFactor") == 0) {
            mi->setParameter(cName, filament::math::float4{r, g, b, a});
        } else {
            mi->setParameter(cName, filament::math::float3{r, g, b});
        }
    });
    env->ReleaseStringUTFChars(name, cName);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialCullMode(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jint cullMode) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    auto mode = static_cast<filament::backend::CullingMode>(cullMode);
    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [mode](filament::MaterialInstance* mi) {
        mi->setCullingMode(mode);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialDepthWrite(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean enable) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [enable](filament::MaterialInstance* mi) {
        mi->setDepthWrite(enable);
    });
}

JNIEXPORT jint JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_getAnimationCount(
        JNIEnv*, jclass, jlong engineHandle, jlong meshHandle) {

    auto* asset = ResourceManagerImpl::getAsset(static_cast<MeshHandle>(meshHandle));
    if (!asset || !asset->getInstance()) return 0;

    auto* animator = asset->getInstance()->getAnimator();
    return animator ? static_cast<jint>(animator->getAnimationCount()) : 0;
}

JNIEXPORT jstring JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_getAnimationName(
        JNIEnv* env, jclass, jlong engineHandle, jlong meshHandle, jint index) {

    auto* asset = ResourceManagerImpl::getAsset(static_cast<MeshHandle>(meshHandle));
    if (!asset || !asset->getInstance()) return env->NewStringUTF("");

    auto* animator = asset->getInstance()->getAnimator();
    if (!animator || index < 0 || index >= static_cast<jint>(animator->getAnimationCount())) {
        return env->NewStringUTF("");
    }

    const char* name = animator->getAnimationName(static_cast<size_t>(index));
    return env->NewStringUTF(name ? name : "");
}

JNIEXPORT jfloat JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_getAnimationDuration(
        JNIEnv*, jclass, jlong engineHandle, jlong meshHandle, jint index) {

    auto* asset = ResourceManagerImpl::getAsset(static_cast<MeshHandle>(meshHandle));
    if (!asset || !asset->getInstance()) return 0.0f;

    auto* animator = asset->getInstance()->getAnimator();
    if (!animator || index < 0 || index >= static_cast<jint>(animator->getAnimationCount())) {
        return 0.0f;
    }

    return animator->getAnimationDuration(static_cast<size_t>(index));
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_applyAnimation(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId,
        jint animationIndex, jfloat timeSeconds,
        jboolean enableCrossFade, jint prevAnimationIndex, jfloat prevTimeSeconds, jfloat alpha) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    entt::entity e = unpackEntity(static_cast<EntityId>(entityId));
    auto& reg = sceneImpl->registry();
    if (!reg.valid(e)) return;

    auto* mc = reg.try_get<components::MeshC>(e);
    if (!mc || mc->meshHandle == INVALID_MESH) return;

    auto* asset = ResourceManagerImpl::getAsset(mc->meshHandle);
    if (!asset || !asset->getInstance()) return;

    auto* animator = asset->getInstance()->getAnimator();
    if (!animator) return;

    if (animationIndex >= 0 && animationIndex < static_cast<jint>(animator->getAnimationCount())) {
        animator->applyAnimation(static_cast<size_t>(animationIndex), timeSeconds);

        if (enableCrossFade && prevAnimationIndex >= 0 && prevAnimationIndex < static_cast<jint>(animator->getAnimationCount())) {
            animator->applyCrossFade(static_cast<size_t>(prevAnimationIndex), prevTimeSeconds, alpha);
        }

        animator->updateBoneMatrices();
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setShadowType(
        JNIEnv*, jclass, jlong engineHandle, jint shadowType) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    rendererImpl->getView()->setShadowType(static_cast<filament::ShadowType>(shadowType));
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setVsmShadowOptions(
        JNIEnv*, jclass, jlong engineHandle,
        jint anisotropy, jboolean mipmapping, jint msaaSamples, jboolean highPrecision, jfloat lightBleedReduction) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::VsmShadowOptions vsm;
    vsm.anisotropy = static_cast<uint8_t>(anisotropy);
    vsm.mipmapping = mipmapping;
    vsm.msaaSamples = static_cast<uint8_t>(msaaSamples);
    vsm.highPrecision = highPrecision;
    vsm.lightBleedReduction = lightBleedReduction;

    rendererImpl->getView()->setVsmShadowOptions(vsm);
}

JNIEXPORT jlongArray JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createInstancedEntity(
        JNIEnv* env, jclass, jlong sceneHandle, jstring name, jlong meshHandle, jfloatArray transformsArray, jint instanceCount) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return nullptr;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    filament::Scene* fScene = sceneImpl->filamentScene();
    if (!fEngine || !fScene) return nullptr;

    auto* entry = ResourceManagerImpl::getEntry(static_cast<MeshHandle>(meshHandle));
    if (!entry) return nullptr;

    const char* cName = env->GetStringUTFChars(name, nullptr);
    EntityId newId = sceneImpl->createEntity(cName);
    env->ReleaseStringUTFChars(name, cName);

    entt::entity e = unpackEntity(newId);
    utils::Entity customEntity = utils::EntityManager::get().create();

    std::vector<filament::math::mat4f> matrices(instanceCount);
    if (transformsArray != nullptr) {
        jfloat* data = env->GetFloatArrayElements(transformsArray, nullptr);
        jsize len = env->GetArrayLength(transformsArray);

        for (int i = 0; i < instanceCount; ++i) {
            if ((i + 1) * 9 <= len) {
                float px = data[i * 9 + 0], py = data[i * 9 + 1], pz = data[i * 9 + 2];
                float rx = data[i * 9 + 3], ry = data[i * 9 + 4], rz = data[i * 9 + 5];
                float sx = data[i * 9 + 6], sy = data[i * 9 + 7], sz = data[i * 9 + 8];

                using namespace filament::math;
                mat4f S = mat4f::scaling(float3{sx, sy, sz});
                mat4f R = mat4f::rotation(ry, float3{0, 1, 0}) *
                          mat4f::rotation(rx, float3{1, 0, 0}) *
                          mat4f::rotation(rz, float3{0, 0, 1});
                mat4f T = mat4f::translation(float3{px, py, pz});

                matrices[i] = T * R * S;
            } else {
                matrices[i] = filament::math::mat4f(1.0f);
            }
        }
        env->ReleaseFloatArrayElements(transformsArray, data, JNI_ABORT);
    } else {
        for (int i = 0; i < instanceCount; ++i) matrices[i] = filament::math::mat4f(1.0f);
    }

    filament::InstanceBuffer* ibuf = filament::InstanceBuffer::Builder(instanceCount)
            .localTransforms(matrices.data())
            .build(*fEngine);

    auto& rcm = fEngine->getRenderableManager();

    if (entry->vertexBuffer && entry->indexBuffer) {
        auto srcInst = rcm.getInstance(entry->customEntity);
        auto* mi = rcm.getMaterialInstanceAt(srcInst, 0);
        auto box = rcm.getAxisAlignedBoundingBox(srcInst);

        filament::RenderableManager::Builder(1)
                .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, entry->vertexBuffer, entry->indexBuffer)
                .material(0, mi)
                .boundingBox(box)
                .instances(static_cast<size_t>(instanceCount), ibuf)
                .build(*fEngine, customEntity);

        fScene->addEntity(customEntity);
    } else if (entry->asset) {
        for (int i = 0; i < instanceCount; ++i) {
            auto inst = ResourceManagerImpl::createInstance(meshHandle, fScene);
            auto tInst = fEngine->getTransformManager().getInstance(inst);
            if (tInst.isValid()) {
                fEngine->getTransformManager().setTransform(tInst, matrices[i]);
            }
        }
    }

    std::vector<filament::MaterialInstance*> instances;
    sceneImpl->registry().emplace_or_replace<components::MeshC>(e, components::MeshC{customEntity, static_cast<MeshHandle>(meshHandle), instances});

    jlongArray res = env->NewLongArray(2);
    jlong handles[2] = { static_cast<jlong>(newId), reinterpret_cast<jlong>(ibuf) };
    env->SetLongArrayRegion(res, 0, 2, handles);
    return res;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_updateInstanceTransforms(
        JNIEnv* env, jclass, jlong instanceBufferHandle, jfloatArray transformsArray, jint count) {

    auto* ibuf = reinterpret_cast<filament::InstanceBuffer*>(instanceBufferHandle);
    if (!ibuf || transformsArray == nullptr) return;

    jfloat* data = env->GetFloatArrayElements(transformsArray, nullptr);
    jsize len = env->GetArrayLength(transformsArray);

    std::vector<filament::math::mat4f> matrices(count);
    for (int i = 0; i < count; ++i) {
        if ((i + 1) * 9 <= len) {
            float px = data[i * 9 + 0], py = data[i * 9 + 1], pz = data[i * 9 + 2];
            float rx = data[i * 9 + 3], ry = data[i * 9 + 4], rz = data[i * 9 + 5];
            float sx = data[i * 9 + 6], sy = data[i * 9 + 7], sz = data[i * 9 + 8];

            using namespace filament::math;
            mat4f S = mat4f::scaling(float3{sx, sy, sz});
            mat4f R = mat4f::rotation(ry, float3{0, 1, 0}) *
                      mat4f::rotation(rx, float3{1, 0, 0}) *
                      mat4f::rotation(rz, float3{0, 0, 1});
            mat4f T = mat4f::translation(float3{px, py, pz});

            matrices[i] = T * R * S;
        }
    }

    ibuf->setLocalTransforms(matrices.data(), count, 0);
    env->ReleaseFloatArrayElements(transformsArray, data, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setSkyboxTexture(
        JNIEnv*, jclass, jlong sceneHandle, jlong textureHandle, jfloat intensity) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    filament::Engine* fEngine = sceneImpl->filamentEngine();
    filament::Scene* fScene = sceneImpl->filamentScene();
    if (!fEngine || !fScene) return;

    auto* texture = reinterpret_cast<filament::Texture*>(textureHandle);
    if (!texture) return;

    if (texture->getTarget() != filament::Texture::Sampler::SAMPLER_CUBEMAP) {
        __android_log_print(ANDROID_LOG_WARN, "RaptorV", "Skybox texture must be a Cubemap!");
        return;
    }

    filament::Skybox* skybox = filament::Skybox::Builder()
            .environment(texture)
            .intensity(intensity)
            .build(*fEngine);

    auto* oldSkybox = fScene->getSkybox();
    if (oldSkybox) fEngine->destroy(oldSkybox);

    fScene->setSkybox(skybox);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createCubemapFromBitmaps(
        JNIEnv* env, jclass, jlong engineHandle, jobjectArray bitmapsArray) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    jsize count = env->GetArrayLength(bitmapsArray);
    if (count < 6) return 0;

    jobject firstBitmap = env->GetObjectArrayElement(bitmapsArray, 0);
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, firstBitmap, &info) < 0) return 0;

    uint32_t width = info.width;
    uint32_t height = info.height;
    size_t faceSize = width * height * 4;
    size_t totalSize = faceSize * 6;

    filament::Texture* cubemap = filament::Texture::Builder()
            .width(width)
            .height(height)
            .levels(1)
            .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
            .format(filament::Texture::InternalFormat::RGBA8)
            .build(*fEngine);

    uint8_t* totalBuffer = new uint8_t[totalSize];

    filament::Texture::FaceOffsets offsets;

    for (int i = 0; i < 6; ++i) {
        offsets[i] = i * faceSize;
        jobject bitmap = env->GetObjectArrayElement(bitmapsArray, i);
        void* pixels = nullptr;
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0) {
            std::memcpy(totalBuffer + (i * faceSize), pixels, faceSize);
            AndroidBitmap_unlockPixels(env, bitmap);
        }
    }

    filament::Texture::PixelBufferDescriptor desc(
            totalBuffer, totalSize,
            filament::Texture::Format::RGBA,
            filament::Texture::Type::UBYTE,
            [](void* buffer, size_t, void*) {
                delete[] static_cast<uint8_t*>(buffer);
            }
    );

    cubemap->setImage(*fEngine, 0, std::move(desc), offsets);

    return reinterpret_cast<jlong>(cubemap);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createTextureFromHdrBytes(
        JNIEnv* env, jclass, jlong engineHandle, jbyteArray hdrBytes, jint size) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    if (!hdrBytes || size <= 0) return 0;

    jbyte* data = env->GetByteArrayElements(hdrBytes, nullptr);

    filament::Texture* texture = filament::Texture::Builder()
            .width(512)
            .height(512)
            .levels(1)
            .sampler(filament::Texture::Sampler::SAMPLER_2D)
            .format(filament::Texture::InternalFormat::R11F_G11F_B10F)
            .build(*fEngine);

    env->ReleaseByteArrayElements(hdrBytes, data, JNI_ABORT);

    return reinterpret_cast<jlong>(texture);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_compileMaterialFromGLSL(
        JNIEnv* env, jclass, jlong engineHandle, jstring glslCode) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    const char* cGlsl = env->GetStringUTFChars(glslCode, nullptr);

    filamat::MaterialBuilder builder;
    builder.name("CustomRuntimeShader")
            .material(cGlsl)
            .targetApi(filamat::MaterialBuilder::TargetApi::ALL);

    filamat::Package package = builder.build(fEngine->getJobSystem());
    env->ReleaseStringUTFChars(glslCode, cGlsl);

    if (!package.isValid()) {
        return 0;
    }

    filament::Material* customMaterial = filament::Material::Builder()
            .package(package.getData(), package.getSize())
            .build(*fEngine);

    return reinterpret_cast<jlong>(customMaterial);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createCubemapFromEquirectangularBitmap(
        JNIEnv* env, jclass, jlong engineHandle, jobject bitmap) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    if (!bitmap) return 0;

    AndroidBitmapInfo info;
    void* srcPixels = nullptr;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0 || AndroidBitmap_lockPixels(env, bitmap, &srcPixels) < 0) {
        return 0;
    }

    uint32_t srcW = info.width;
    uint32_t srcH = info.height;
    uint32_t* srcData = static_cast<uint32_t*>(srcPixels);

    uint32_t faceDim = 512;
    size_t faceSize = faceDim * faceDim * 4;
    size_t totalSize = faceSize * 6;

    uint8_t* totalBuffer = new uint8_t[totalSize];
    uint32_t* dstData = reinterpret_cast<uint32_t*>(totalBuffer);

    filament::Texture::FaceOffsets offsets;

    for (int face = 0; face < 6; ++face) {
        offsets[face] = face * faceSize;
        uint32_t* faceBuffer = dstData + (face * faceDim * faceDim);

        for (uint32_t y = 0; y < faceDim; ++y) {
            float v = (2.0f * (y + 0.5f) / faceDim) - 1.0f;

            for (uint32_t x = 0; x < faceDim; ++x) {
                float u = (2.0f * (x + 0.5f) / faceDim) - 1.0f;

                float vx = 0, vy = 0, vz = 0;
                switch (face) {
                    case 0: vx =  1.0f; vy = -v;    vz = -u;    break; // Right (+X)
                    case 1: vx = -1.0f; vy = -v;    vz =  u;    break; // Left (-X)
                    case 2: vx =  u;    vy =  1.0f; vz =  v;    break; // Top (+Y)
                    case 3: vx =  u;    vy = -1.0f; vz = -v;    break; // Bottom (-Y)
                    case 4: vx =  u;    vy = -v;    vz =  1.0f; break; // Back (+Z)
                    case 5: vx = -u;    vy = -v;    vz = -1.0f; break; // Front (-Z)
                }

                float radius = sqrtf(vx * vx + vy * vy + vz * vz);
                float phi = atan2f(vz, vx);
                float theta = asinf(vy / radius);

                float texU = (phi / (2.0f * M_PI)) + 0.5f;
                float texV = (theta / M_PI) + 0.5f;

                uint32_t srcX = static_cast<uint32_t>(texU * srcW) % srcW;
                uint32_t srcY = static_cast<uint32_t>((1.0f - texV) * srcH) % srcH;

                faceBuffer[y * faceDim + x] = srcData[srcY * srcW + srcX];
            }
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    filament::Texture* cubemap = filament::Texture::Builder()
            .width(faceDim)
            .height(faceDim)
            .levels(1)
            .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
            .format(filament::Texture::InternalFormat::RGBA8)
            .build(*fEngine);

    filament::Texture::PixelBufferDescriptor desc(
            totalBuffer, totalSize,
            filament::Texture::Format::RGBA,
            filament::Texture::Type::UBYTE,
            [](void* buffer, size_t, void*) { delete[] static_cast<uint8_t*>(buffer); }
    );

    cubemap->setImage(*fEngine, 0, std::move(desc), offsets);

    return reinterpret_cast<jlong>(cubemap);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createCubemapFromHdrBytes(
        JNIEnv* env, jclass, jlong engineHandle, jbyteArray hdrBytes, jint size) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    if (!hdrBytes || size <= 0) return 0;

    jbyte* data = env->GetByteArrayElements(hdrBytes, nullptr);

    int srcW = 0, srcH = 0;
    std::vector<float> hdrPixels;
    bool ok = parseHDRBuffer(reinterpret_cast<const uint8_t*>(data), static_cast<size_t>(size), srcW, srcH, hdrPixels);

    env->ReleaseByteArrayElements(hdrBytes, data, JNI_ABORT);

    if (!ok || srcW <= 0 || srcH <= 0) {
        __android_log_print(ANDROID_LOG_ERROR, "RaptorV", "Failed to parse HDR file buffer in C++!");
        return 0;
    }

    uint32_t faceDim = 512;
    size_t faceSize = faceDim * faceDim * 3 * sizeof(float);
    size_t totalSize = faceSize * 6;

    uint8_t* totalBuffer = new uint8_t[totalSize];
    float* dstData = reinterpret_cast<float*>(totalBuffer);

    filament::Texture::FaceOffsets offsets;

    for (int face = 0; face < 6; ++face) {
        offsets[face] = face * faceSize;
        float* faceBuffer = dstData + (face * faceDim * faceDim * 3);

        for (uint32_t y = 0; y < faceDim; ++y) {
            float v = (2.0f * (y + 0.5f) / faceDim) - 1.0f;

            for (uint32_t x = 0; x < faceDim; ++x) {
                float u = (2.0f * (x + 0.5f) / faceDim) - 1.0f;

                float vx = 0, vy = 0, vz = 0;
                switch (face) {
                    case 0: vx =  1.0f; vy = -v;    vz = -u;    break; // Right (+X)
                    case 1: vx = -1.0f; vy = -v;    vz =  u;    break; // Left (-X)
                    case 2: vx =  u;    vy =  1.0f; vz =  v;    break; // Top (+Y)
                    case 3: vx =  u;    vy = -1.0f; vz = -v;    break; // Bottom (-Y)
                    case 4: vx =  u;    vy = -v;    vz =  1.0f; break; // Back (+Z)
                    case 5: vx = -u;    vy = -v;    vz = -1.0f; break; // Front (-Z)
                }

                float radius = sqrtf(vx * vx + vy * vy + vz * vz);
                float phi = atan2f(vz, vx);
                float theta = asinf(vy / radius);

                float texU = (phi / (2.0f * M_PI)) + 0.5f;
                float texV = (theta / M_PI) + 0.5f;

                int srcX = static_cast<int>(texU * srcW) % srcW;
                int srcY = static_cast<int>((1.0f - texV) * srcH) % srcH;
                if (srcX < 0) srcX += srcW;
                if (srcY < 0) srcY += srcH;

                int srcIndex = (srcY * srcW + srcX) * 3;
                int dstIndex = (y * faceDim + x) * 3;

                faceBuffer[dstIndex + 0] = hdrPixels[srcIndex + 0];
                faceBuffer[dstIndex + 1] = hdrPixels[srcIndex + 1];
                faceBuffer[dstIndex + 2] = hdrPixels[srcIndex + 2];
            }
        }
    }

    filament::Texture* cubemap = filament::Texture::Builder()
            .width(faceDim)
            .height(faceDim)
            .levels(1)
            .sampler(filament::Texture::Sampler::SAMPLER_CUBEMAP)
            .format(filament::Texture::InternalFormat::R11F_G11F_B10F)
            .build(*fEngine);

    filament::Texture::PixelBufferDescriptor desc(
            totalBuffer, totalSize,
            filament::Texture::Format::RGB,
            filament::Texture::Type::FLOAT,
            [](void* buffer, size_t, void*) { delete[] static_cast<uint8_t*>(buffer); }
    );

    cubemap->setImage(*fEngine, 0, std::move(desc), offsets);

    return reinterpret_cast<jlong>(cubemap);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setLightPositionAndDirection(
        JNIEnv*, jclass, jlong engineHandle, jlong lightId,
        jfloat px, jfloat py, jfloat pz, jfloat dx, jfloat dy, jfloat dz) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->engine()) return;

    entt::entity e = unpackEntity(static_cast<EntityId>(lightId));
    auto* activeScene = rendererImpl->getActiveScene();
    if (!activeScene) return;

    auto* lc = activeScene->registry().try_get<components::LightC>(e);
    if (!lc) return;

    auto& lm = rendererImpl->engine()->getLightManager();
    auto fLight = lm.getInstance(lc->filamentEntity);
    if (!fLight.isValid()) return;

    lm.setPosition(fLight, {px, py, pz});
    lm.setDirection(fLight, {dx, dy, dz});
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setLightChannel(
        JNIEnv*, jclass, jlong engineHandle, jlong lightId, jint channel, jboolean enable) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->engine()) return;

    entt::entity e = unpackEntity(static_cast<EntityId>(lightId));
    auto* activeScene = rendererImpl->getActiveScene();
    if (!activeScene) return;

    auto* lc = activeScene->registry().try_get<components::LightC>(e);
    if (!lc) return;

    auto& lm = rendererImpl->engine()->getLightManager();
    auto fLight = lm.getInstance(lc->filamentEntity);
    if (!fLight.isValid()) return;

    lm.setLightChannel(fLight, static_cast<unsigned int>(channel), enable);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityLightChannel(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jint channel, jboolean enable) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    uint8_t ch = static_cast<uint8_t>(std::clamp(channel, 0, 7));
    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [ch, enable](auto& rcm, auto inst) {
        rcm.setLightChannel(inst, ch, enable);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMorphTargetWeights(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jfloatArray weightsArray) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    if (!weightsArray) return;

    jsize count = env->GetArrayLength(weightsArray);
    jfloat* weightsData = env->GetFloatArrayElements(weightsArray, nullptr);

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [weightsData, count](auto& rcm, auto inst) {
        rcm.setMorphWeights(inst, weightsData, static_cast<size_t>(count));
    });

    env->ReleaseFloatArrayElements(weightsArray, weightsData, JNI_ABORT);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setGuardBandOptions(
        JNIEnv*, jclass, jlong engineHandle, jboolean enabled) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());

    filament::GuardBandOptions gb;
    gb.enabled = enabled;
    rendererImpl->getView()->setGuardBandOptions(gb);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createSkinningBuffer(
        JNIEnv*, jclass, jlong engineHandle, jint boneCount) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    if (!wrapper || reinterpret_cast<uintptr_t>(wrapper) < 0x10000 || !wrapper->engine) return 0;

    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    filament::SkinningBuffer* sb = filament::SkinningBuffer::Builder()
            .boneCount(static_cast<uint32_t>(boneCount))
            .build(*fEngine);

    return reinterpret_cast<jlong>(sb);
}


JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_destroySkinningBuffer(
        JNIEnv*, jclass, jlong engineHandle, jlong skinningBufferHandle) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    filament::Engine* fEngine = rendererImpl->engine();

    auto* sb = reinterpret_cast<filament::SkinningBuffer*>(skinningBufferHandle);
    if (sb) fEngine->destroy(sb);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntitySkinningBuffer(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jlong skinningBufferHandle, jint count, jint offset) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    auto* sb = reinterpret_cast<filament::SkinningBuffer*>(skinningBufferHandle);
    if (!sb) return;

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [sb, count, offset](auto& rcm, auto inst) {
        rcm.setSkinningBuffer(inst, sb, static_cast<size_t>(count), static_cast<size_t>(offset));
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityMaterialParamAt(
        JNIEnv* env, jclass, jlong sceneHandle, jlong entityId, jint primitiveIndex, jstring paramName, jfloat value) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    const char* cName = env->GetStringUTFChars(paramName, nullptr);

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [primitiveIndex, cName, value](auto& rcm, auto inst) {
        if (static_cast<size_t>(primitiveIndex) < rcm.getPrimitiveCount(inst)) {
            auto* mi = rcm.getMaterialInstanceAt(inst, static_cast<size_t>(primitiveIndex));
            if (mi) {
                mi->setParameter(cName, value);
            }
        }
    });

    env->ReleaseStringUTFChars(paramName, cName);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setViewVisibleLayers(
        JNIEnv*, jclass, jlong engineHandle, jint select, jint values) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (rendererImpl && rendererImpl->getView()) {
        rendererImpl->getView()->setVisibleLayers(static_cast<uint8_t>(select), static_cast<uint8_t>(values));
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityBlendOrderAt(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jint primitiveIndex, jint order) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [primitiveIndex, order](auto& rcm, auto inst) {
        if (static_cast<size_t>(primitiveIndex) < rcm.getPrimitiveCount(inst)) {
            rcm.setBlendOrderAt(inst, static_cast<size_t>(primitiveIndex), static_cast<uint16_t>(order));
        }
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityBoundingBox(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId,
        jfloat cx, jfloat cy, jfloat cz, jfloat ex, jfloat ey, jfloat ez) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    filament::Box box{ {cx, cy, cz}, {ex, ey, ez} };

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [box](auto& rcm, auto inst) {
        rcm.setAxisAlignedBoundingBox(inst, box);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setIndirectLightRotation3D(
        JNIEnv*, jclass, jlong sceneHandle, jfloat rx, jfloat ry, jfloat rz) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    sceneImpl->setIndirectLightRotation3D(rx, ry, rz);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityFogEnabled(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean enabled) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToRenderableInstances(sceneImpl, static_cast<EntityId>(entityId), [enabled](auto& rcm, auto inst) {
        rcm.setFogEnabled(inst, enabled);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setEntityPolygonOffset(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jfloat factor, jfloat units) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [factor, units](filament::MaterialInstance* mi) {
        mi->setPolygonOffset(factor, units);
    });
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setPostProcessingEnabled(
        JNIEnv*, jclass, jlong engineHandle, jboolean enabled) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (rendererImpl && rendererImpl->getView()) {
        rendererImpl->getView()->setPostProcessingEnabled(enabled);
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setSkyboxShowSun(
        JNIEnv*, jclass, jlong sceneHandle, jboolean showSun) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    sceneImpl->setSkyboxShowSun(showSun);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setViewFrontFaceWindingInverted(
        JNIEnv*, jclass, jlong engineHandle, jboolean inverted) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (rendererImpl && rendererImpl->getView()) {
        rendererImpl->getView()->setFrontFaceWindingInverted(inverted);
    }
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_pickEntityAt(
        JNIEnv*, jclass, jlong engineHandle, jint x, jint y) {

    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    if (!wrapper || reinterpret_cast<uintptr_t>(wrapper) < 0x10000 || !wrapper->engine) return 0;

    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->getView()) return 0;

    return 0;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setAntiAliasing(
        JNIEnv*, jclass, jlong engineHandle, jint mode) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (rendererImpl && rendererImpl->getView()) {
        rendererImpl->getView()->setAntiAliasing(static_cast<filament::View::AntiAliasing>(mode));
    }
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setNightAdaptation(
        JNIEnv*, jclass, jlong engineHandle, jfloat adaptation) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->getView()) return;

    filament::Engine* fEngine = rendererImpl->engine();
    filament::View* view = rendererImpl->getView();

    filament::ColorGrading* colorGrading = filament::ColorGrading::Builder()
            .nightAdaptation(adaptation)
            .build(*fEngine);

    auto* oldColorGrading = const_cast<filament::ColorGrading*>(view->getColorGrading());
    view->setColorGrading(colorGrading);
    if (oldColorGrading) fEngine->destroy(oldColorGrading);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMSAAOptions(
        JNIEnv*, jclass, jlong engineHandle, jboolean enabled, jint sampleCount) {

    CHECK_ENGINE(engineHandle);
    auto* rendererImpl = static_cast<RendererImpl*>(wrapper->engine->getRenderer());
    if (!rendererImpl || !rendererImpl->getView()) return;

    filament::MultiSampleAntiAliasingOptions msaa = rendererImpl->getView()->getMultiSampleAntiAliasingOptions();
    msaa.enabled = enabled;
    msaa.sampleCount = static_cast<uint8_t>(sampleCount);

    rendererImpl->getView()->setMultiSampleAntiAliasingOptions(msaa);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMaterialSpecularAntiAliasing(
        JNIEnv*, jclass, jlong sceneHandle, jlong entityId, jboolean enabled, jfloat variance, jfloat threshold) {

    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    if (!scenePtr || !*scenePtr) return;
    auto* sceneImpl = static_cast<SceneImpl*>(scenePtr->get());

    applyToMaterialInstances(sceneImpl, static_cast<EntityId>(entityId), [variance, threshold](filament::MaterialInstance* mi) {
        mi->setSpecularAntiAliasingVariance(variance);
        mi->setSpecularAntiAliasingThreshold(threshold);
    });
}

} // extern "C"
