#include <jni.h>
#include "../core/RaptorEngine.h"

RaptorEngine engine;

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_initEngine(JNIEnv* env, jobject /* this */) {
    engine.init();
}

extern "C" JNIEXPORT void JNICALL
Java_com_danvexteam_raptorv_RaptorNative_updateEngine(JNIEnv* env, jobject /* this */, jfloat deltaTime) {
    engine.update(deltaTime);
}