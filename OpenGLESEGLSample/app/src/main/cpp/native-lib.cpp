#include <jni.h>
#include <string>
#include <LogAndroid.h>
#include <common.h>
#include <MyDefineUtils.h>
#include "EGLHelper.h"
#include <android/native_window_jni.h>

int m_ImgBufferLength = 0;
unsigned char* m_pImgData = nullptr;

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_opengleseglsample_NativeEGLHelper_Init(JNIEnv *env, jobject clazz)
{
    // TODO: implement Init()
    int retCode = 0;
    if (EGLHelper::CreateInstance()) {
        retCode = EGLHelper::CreateInstance()->Init();
        LOGD ("Java_com_example_opengleseglsample_NativeEGLHelper_Init Init ret = %d", retCode);
    }
    return retCode;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_opengleseglsample_NativeEGLHelper_UnInit(JNIEnv *env, jobject clazz)
{
    // TODO: implement UnInit()
    int retCode = 0;
    if (EGLHelper::CreateInstance()) {
        retCode = EGLHelper::CreateInstance()->UnInit();
        LOGD ("Java_com_example_opengleseglsample_NativeEGLHelper_UnInit UnInit ret = %d", retCode);
    }
    if (nullptr != m_pImgData) {
        SafeFree(m_pImgData);
        m_ImgBufferLength = 0;
    }

    return retCode;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_opengleseglsample_NativeEGLHelper_Draw(JNIEnv *env, jobject clazz)
{
    // TODO: implement Draw()
    int retCode = 0;
    if (EGLHelper::CreateInstance()) {
        retCode = EGLHelper::CreateInstance()->Draw();
        LOGD ("Java_com_example_opengleseglsample_NativeEGLHelper_Draw Draw ret = %d", retCode);
    }
    return retCode;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_opengleseglsample_NativeEGLHelper_SetImageData(JNIEnv *env, jobject clazz,
        jbyteArray data, jint img_width, jint img_height, jint format)
{
    // TODO: implement SetImageData()
    int retCode = 0;
    int width = img_width;
    int height = img_height;

    if (nullptr == data) {
        return ERROR_INPUT;
    }

    int length = env->GetArrayLength(data);
    if (m_ImgBufferLength != length && nullptr != m_pImgData) {
        free(m_pImgData);
        m_pImgData = nullptr;
        m_pImgData = (unsigned char*)malloc(m_ImgBufferLength);
    }
    m_ImgBufferLength = length;

    if (nullptr == m_pImgData) {
        m_pImgData = (unsigned char*)malloc(m_ImgBufferLength);
    }
    jbyte *bytes = env->GetByteArrayElements(data, 0);
    memcpy(m_pImgData, bytes, m_ImgBufferLength);
    env->ReleaseByteArrayElements(data, bytes, 0);

    if (EGLHelper::CreateInstance()) {
        retCode = EGLHelper::CreateInstance()->SetImageData(width, height, m_pImgData);
        LOGD ("Java_com_example_opengleseglsample_NativeEGLHelper_SetImageData Draw ret = %d", retCode);
    }
    return retCode;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_opengleseglsample_NativeEGLHelper_SetWindow(JNIEnv *env, jobject thiz, jobject surface, jint width, jint height)
{
    // TODO: implement SetWindow()

    ANativeWindow *pNativeWindow = ANativeWindow_fromSurface (env, surface);
    EGLHelper::CreateInstance()->SetWindow(pNativeWindow);

    return 0;
}