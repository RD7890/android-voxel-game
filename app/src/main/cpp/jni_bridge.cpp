#include <jni.h>
#include "engine/Renderer.h"
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"MCME",__VA_ARGS__)

static Renderer* gRenderer = nullptr;

extern "C" {

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onSurfaceCreated(JNIEnv*, jobject, jint width, jint height) {
    if (gRenderer) { gRenderer->destroy(); delete gRenderer; }
    gRenderer = new Renderer();
    gRenderer->init(width, height);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onSurfaceChanged(JNIEnv*, jobject, jint width, jint height) {
    if (gRenderer) gRenderer->resize(width, height);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onDrawFrame(JNIEnv*, jobject) {
    if (gRenderer) gRenderer->draw();
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onJoystick(JNIEnv*, jobject, jfloat rightAxis, jfloat forwardAxis) {
    if (gRenderer) gRenderer->onJoystick(rightAxis, forwardAxis);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onLook(JNIEnv*, jobject, jfloat dyaw, jfloat dpitch) {
    if (gRenderer) gRenderer->onLook(dyaw, dpitch);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onJump(JNIEnv*, jobject) {
    if (gRenderer) gRenderer->onJump();
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onPause(JNIEnv*, jobject) {}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_onResume(JNIEnv*, jobject) {}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_setSensitivity(JNIEnv*, jobject, jfloat s) {
    if (gRenderer) gRenderer->setSensitivity(s);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_setRenderDistance(JNIEnv*, jobject, jint d) {
    if (gRenderer) gRenderer->setRenderDistance(d);
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_destroy(JNIEnv*, jobject) {
    if (gRenderer) { gRenderer->destroy(); delete gRenderer; gRenderer = nullptr; }
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_breakBlock(JNIEnv*, jobject) {
    if (gRenderer) gRenderer->breakBlock();
}

JNIEXPORT void JNICALL
Java_com_mcme_NativeLib_placeBlock(JNIEnv*, jobject, jint blockType) {
    if (gRenderer) gRenderer->placeBlock((int)blockType);
}

} // extern "C"
