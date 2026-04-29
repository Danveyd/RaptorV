#include <jni.h>
#include "raptor/Engine.h"
#include "../src/platform/android/AndroidFileSystem.h"
#include "../src/platform/android/AndroidWindow.h"
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <memory>

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
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);

    AndroidWindow aw(window, ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
    wrapper->engine->onSurfaceCreated(&aw);

    ANativeWindow_release(window);
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
Java_com_danvexteam_raptorv_internal_RaptorNative_attachRigidBody(JNIEnv* env, jclass, jlong sceneHandle, jlong entityId,
                                                                  jfloat hx, jfloat hy, jfloat hz, jfloat mass) {
    auto* scenePtr = reinterpret_cast<std::shared_ptr<Scene>*>(sceneHandle);
    RigidBodyDesc desc;
    desc.halfExtent = {hx, hy, hz};
    desc.mass = mass;
    (*scenePtr)->attachRigidBody(static_cast<EntityId>(entityId), desc);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createCamera(JNIEnv* env, jclass, jlong engineHandle,
                                                               jfloat fov, jfloat near, jfloat far,
                                                               jfloat px, jfloat py, jfloat pz,
                                                               jfloat tx, jfloat ty, jfloat tz,
                                                               jfloat ux, jfloat uy, jfloat uz) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    CameraDesc desc;
    desc.fovDegrees = fov; desc.nearPlane = near; desc.farPlane = far;
    desc.position = {px, py, pz};
    desc.target = {tx, ty, tz};
    desc.up = {ux, uy, uz};
    EntityId id = wrapper->engine->getRenderer()->createCamera(desc);
    return static_cast<jlong>(id);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setMainCamera(JNIEnv* env, jclass, jlong engineHandle, jlong cameraId) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    wrapper->engine->getRenderer()->setMainCamera(static_cast<EntityId>(cameraId));
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_updateCamera(JNIEnv* env, jclass, jlong engineHandle, jlong cameraId,
                                                               jfloat fov, jfloat near, jfloat far,
                                                               jfloat px, jfloat py, jfloat pz,
                                                               jfloat tx, jfloat ty, jfloat tz,
                                                               jfloat ux, jfloat uy, jfloat uz) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    CameraDesc desc;
    desc.fovDegrees = fov; desc.nearPlane = near; desc.farPlane = far;
    desc.position = {px, py, pz};
    desc.target = {tx, ty, tz};
    desc.up = {ux, uy, uz};
    wrapper->engine->getRenderer()->updateCamera(static_cast<EntityId>(cameraId), desc);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_setRenderSettings(JNIEnv* env, jclass, jlong engineHandle,
                                                                    jboolean bloom, jboolean ssao, jboolean fxaa, jboolean taa,
                                                                    jfloat cr, jfloat cg, jfloat cb, jfloat ca) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    RenderSettings s;
    s.bloom = bloom; s.ssao = ssao; s.fxaa = fxaa; s.taa = taa;
    s.clearColor = {cr, cg, cb, ca};
    wrapper->engine->getRenderer()->setRenderSettings(s);
}

JNIEXPORT jlong JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_createLight(JNIEnv* env, jclass, jlong engineHandle,
                                                              jint type, jfloat cx, jfloat cy, jfloat cz, jfloat intensity,
                                                              jfloat dx, jfloat dy, jfloat dz, jfloat px, jfloat py, jfloat pz,
                                                              jfloat falloff, jboolean shadows) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    LightDesc desc;
    desc.type = static_cast<LightType>(type);
    desc.color = {cx, cy, cz};
    desc.intensity = intensity;
    desc.direction = {dx, dy, dz};
    desc.position = {px, py, pz};
    desc.falloffRadius = falloff;
    desc.castShadows = shadows;
    EntityId id = wrapper->engine->getRenderer()->createLight(desc);
    return static_cast<jlong>(id);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_internal_RaptorNative_updateLight(JNIEnv* env, jclass, jlong engineHandle, jlong lightId,
                                                              jint type, jfloat cx, jfloat cy, jfloat cz, jfloat intensity,
                                                              jfloat dx, jfloat dy, jfloat dz, jfloat px, jfloat py, jfloat pz,
                                                              jfloat falloff, jboolean shadows) {
    auto* wrapper = reinterpret_cast<EngineWrapper*>(engineHandle);
    LightDesc desc;
    desc.type = static_cast<LightType>(type);
    desc.color = {cx, cy, cz};
    desc.intensity = intensity;
    desc.direction = {dx, dy, dz};
    desc.position = {px, py, pz};
    desc.falloffRadius = falloff;
    desc.castShadows = shadows;
    wrapper->engine->getRenderer()->updateLight(static_cast<EntityId>(lightId), desc);
}

} // extern "C"
