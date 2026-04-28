#include <jni.h>
#include "../core/RaptorEngine.h"
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

extern "C" {

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_initEngine(JNIEnv* env, jobject, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    RaptorEngine::get().init(mgr);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_updateEngine(JNIEnv* env, jobject, jfloat deltaTime) {
    RaptorEngine::get().update(deltaTime);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_onSurfaceCreated(JNIEnv* env, jobject, jobject surface) {
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    RaptorEngine::get().onSurfaceCreated(window);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_onSurfaceChanged(JNIEnv* env, jobject, jint width, jint height) {
    RaptorEngine::get().onSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_onSurfaceDestroyed(JNIEnv* env, jobject) {
    RaptorEngine::get().onSurfaceDestroyed();
}

// --- ECS API ---

JNIEXPORT jint JNICALL
Java_com_danvexteam_raptorv_RaptorNative_createEntity(JNIEnv* env, jobject, jstring name) {
    const char* nativeName = env->GetStringUTFChars(name, nullptr);
    uint32_t id = RaptorEngine::get().createEntity(nativeName);
    env->ReleaseStringUTFChars(name, nativeName);
    return id;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_destroyEntity(JNIEnv* env, jobject, jint entityId) {
    RaptorEngine::get().destroyEntity(entityId);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_setPosition(JNIEnv* env, jobject, jint entityId, jfloat x, jfloat y, jfloat z) {
    RaptorEngine::get().setPosition(entityId, x, y, z);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_setScale(JNIEnv* env, jobject, jint entityId, jfloat x, jfloat y, jfloat z) {
    RaptorEngine::get().setScale(entityId, x, y, z);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_loadGlTF(JNIEnv* env, jobject, jint entityId, jstring path) {
    const char* nativePath = env->GetStringUTFChars(path, nullptr);
    RaptorEngine::get().loadGlTF(entityId, nativePath);
    env->ReleaseStringUTFChars(path, nativePath);
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_setRotation(JNIEnv* env, jobject, jint entityId, jfloat x, jfloat y, jfloat z) {
    RaptorEngine::get().setRotation(entityId, x, y, z);
}

JNIEXPORT jfloatArray JNICALL
Java_com_danvexteam_raptorv_RaptorNative_getPosition(JNIEnv* env, jobject, jint entityId) {
    auto pos = RaptorEngine::get().getPosition(entityId);
    jfloatArray result = env->NewFloatArray(3);
    float arr[3] = {pos.x, pos.y, pos.z};
    env->SetFloatArrayRegion(result, 0, 3, arr);
    return result;
}

JNIEXPORT jfloatArray JNICALL
Java_com_danvexteam_raptorv_RaptorNative_getRotation(JNIEnv* env, jobject, jint entityId) {
    auto rot = RaptorEngine::get().getRotation(entityId);
    jfloatArray result = env->NewFloatArray(3);
    float arr[3] = {rot.x, rot.y, rot.z};
    env->SetFloatArrayRegion(result, 0, 3, arr);
    return result;
}

JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_addBoxCollider(JNIEnv* env, jobject, jint entityId, jfloat hx, jfloat hy, jfloat hz, jfloat mass) {
    RaptorEngine::get().addBoxCollider(entityId, hx, hy, hz, mass);
}

} // extern "C"
