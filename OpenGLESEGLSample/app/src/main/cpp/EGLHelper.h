//
// Created by chauncy on 2020/7/30.
//

#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <MyImageInfo.h>
#include "ShaderHelper.h"

#define GL_VBO_MAX	5

// RGB color values for generated frames
#define TEST_R0		0
#define TEST_G0 	136
#define TEST_B0 	0
#define TEST_R1 	236
#define TEST_G1 	50
#define TEST_B1 	186

class EGLHelper
{
public:
	// 单例
	static EGLHelper* CreateInstance ();
	int SetWindow (ANativeWindow * const pNativeWindow, const int windowWith, const int windowHeight);
	int SetWindowRender (ANativeWindow * const pNativeWindow, const int windowWith, const int windowHeight);
	int Init(int usage = 0, ANativeWindow *const pWindow = nullptr); // usage: 0 for render, 1 for encode
	int UnInit ();
	int Draw ();
	bool SwapBuffers();
	void SetPresentationTime(long nsecs);
	int SetRecordWindow (ANativeWindow * const pWindow);
	int SetImageData (const int imgWidth, const int imgHeight, const unsigned char* pImgData);

private:
	EGLHelper ();
	~EGLHelper ();
	int CreateEGLEnv (int usage, ANativeWindow *const pWindow);
	int DestroyEGLEnv ();

	int createShader();
	void destroyShader();
	int creteGLBuffer ();
	void destroyGLBuffer ();
	int drawFBO();

	static EGLHelper *m_EGLHelper;
	EGLDisplay m_EGLDisplay;
	EGLConfig m_EGLConfig;
	EGLSurface m_EGLSurface;
	EGLSurface m_EGLSurfaceRender;
	EGLContext m_EGLContext;
	bool m_bEGLEnvReady;
	ANativeWindow *m_pRecordWindow;
	PFNEGLPRESENTATIONTIMEANDROIDPROC m_pPresentTimeFunc;

	ANativeWindow *m_pANativeWindow;
	ANativeWindow *m_pANativeWindowRender;
	PFNEGLPRESENTATIONTIMEANDROIDPROC m_pfneglPresentationTimeANDROID;

	GLuint m_VAO;
	GLuint m_VBO;
	//GLuint m_VBO[GL_VBO_MAX];
	GLuint m_EBO;
	GLuint m_FBO;
	GLuint m_FBOTexture;
	ShaderHelper* m_pShaderHelperNormal;
	ShaderHelper* m_pShaderHelperFBO;

	int m_WindowWidth;
	int m_WindowHeight;

	int m_VideoWidth;
	int m_VideoHeight;
	long m_lBeginTime;

	int m_ImgFormat;
	MyImageInfo m_RenderImg;

	int m_FrameID;
};



