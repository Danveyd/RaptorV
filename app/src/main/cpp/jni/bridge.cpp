#include <jni.h>
#include "../core/RaptorEngine.h"
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

RaptorEngine engine;

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_initEngine(JNIEnv* env, jobject /* this */, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    engine.init(mgr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_updateEngine(JNIEnv* env, jobject /* this */, jfloat deltaTime) {
    engine.update(deltaTime);
}

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_onSurfaceCreated(JNIEnv* env, jobject /* this */, jobject surface) {
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    engine.onSurfaceCreated(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_onSurfaceDestroyed(JNIEnv* env, jobject /* this */) {
    engine.onSurfaceDestroyed();
}