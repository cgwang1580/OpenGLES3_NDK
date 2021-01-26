//
// Created by chauncy on 2021/1/26.
//

#pragma once

#include "jni.h"

extern "C" {
	///---------------------------------- NativeEGLHelper function ------------------------------------///
	JNIEXPORT jint JNICALL Java_com_example_opengleseglsample_NativeEGLHelper_Init(JNIEnv *env, jobject clazz);
	JNIEXPORT jint JNICALL Java_com_example_opengleseglsample_NativeEGLHelper_UnInit(JNIEnv *env, jobject clazz);
	JNIEXPORT jint JNICALL Java_com_example_opengleseglsample_NativeEGLHelper_Draw(JNIEnv *env, jobject clazz);
	JNIEXPORT jint JNICALL Java_com_example_opengleseglsample_NativeEGLHelper_SetImageData(JNIEnv *env,
			jobject clazz, jbyteArray data, jint img_width, jint img_height, jint format);

	///---------------------------------- OpenSLESActivity function ------------------------------------///
	JNIEXPORT jint JNICALL Java_com_example_opengleseglsample_OpenSLESActivity_createSLEngine(JNIEnv *env,
			jobject thiz);
	JNIEXPORT jboolean JNICALL Java_com_example_opengleseglsample_OpenSLESActivity_createAssetAudioPlayer(JNIEnv *env,
			jobject thiz, jobject asset_manager, jstring file_name);
	JNIEXPORT void JNICALL Java_com_example_opengleseglsample_OpenSLESActivity_setPlayingAssetAudioPlayerState(JNIEnv *env,
																											   jobject thiz, jboolean b_play);
	JNIEXPORT void JNICALL Java_com_example_opengleseglsample_OpenSLESActivity_destroySLEngine(JNIEnv *env, jobject thiz);
}