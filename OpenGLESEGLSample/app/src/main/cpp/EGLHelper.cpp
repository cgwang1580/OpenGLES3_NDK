//
// Created by chauncy on 2020/7/30.
//

#include <common.h>
#include <MyDefineUtils.h>
#include <cstring>
#include "EGLHelper.h"
#include "LogAndroid.h"
#include "DrawHelper.h"

#define GLES_VERSION_STRING		"#version 300 es\n"
#define GLES_MEDIUM_STRING		"precision mediump float;\n"

// points vertex
const GLfloat vertex[] = {
	1.0f, -1.0f, 0.f,
	1.0f,  1.0f, 0.f,
	-1.0f, 1.0f, 0.f,
	-1.0f, -1.0f, 0.f
};

// 正常纹理坐标
const GLfloat vTexCoors[] = {
		0.f, 1.f,
		1.f, 1.f,
		0.f, 1.f,
		0.f, 0.f,
};

// FBO纹理坐标，与正常纹理坐标倒过来
const GLfloat vTexCoorsFBO[] = {
		1.f, 1.f,
		0.f, 1.f,
		0.f, 0.f,
		0.f, 1.f,
};

const char *triangle_vertex_shader0 =
		GLES_VERSION_STRING
R"(
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main()
{
	gl_Position = mvp * vec4(aPos, 1.0);
}
)";

const char *triangle_fragment_shader0 =
		GLES_VERSION_STRING
GLES_MEDIUM_STRING
R"(
out vec4 FragColor;
void main()
{
	FragColor = vec4(1.0f, 0.0f, 0.2f, 1.0f);
}
)";

const char *triangle_fragment_shader1 =
		GLES_VERSION_STRING
		GLES_MEDIUM_STRING
		R"(
out vec4 FragColor;
void main()
{
	FragColor = vec4(0.0f, 1.0f, 0.2f, 1.0f);
}
)";

EGLHelper* EGLHelper::m_EGLHelper = nullptr;

EGLHelper *EGLHelper::CreateInstance()
{
	if (nullptr == EGLHelper::m_EGLHelper) {
		EGLHelper::m_EGLHelper = new EGLHelper();
	}
	return EGLHelper::m_EGLHelper;
}

int EGLHelper::Init(int usage, ANativeWindow *const pWindow)
{
	LOGD("EGLHelper::Init");

	int retCode = ERROR_OK;
	do {

		retCode = CreateEGLEnv(usage, pWindow);
		if (ERROR_OK != retCode)
			break;

		retCode = createShader();
		CHECK_OK_RETURN(retCode);
		retCode = creteGLBuffer();
		CHECK_OK_RETURN(retCode);

	} while (false);

	return retCode;
}

int EGLHelper::UnInit()
{
	LOGD("EGLHelper::UnInit");

	destroyShader();
	destroyGLBuffer ();
	DestroyEGLEnv();

	return ERROR_OK;
}

EGLHelper::EGLHelper():
		m_EGLDisplay (nullptr),
		m_EGLConfig (nullptr),
		m_EGLSurface(nullptr),
		m_EGLContext(nullptr),
		m_pShaderHelperNormal (nullptr),
		m_pShaderHelperFBO (nullptr),
		m_pRecordWindow(nullptr),
		m_bEGLEnvReady(false),
		m_pPresentTimeFunc(nullptr),
		m_lBeginTime(0)
{
	m_VAO = GL_NONE;
	/*for (auto val : m_VBO)
		val = GL_NONE;*/
	m_VBO = GL_NONE;
	m_EBO = GL_NONE;
	m_FBO = GL_NONE;
	m_FBOTexture = GL_NONE;
	m_FrameID = 0;

	memset(&m_RenderImg, 0, sizeof(MyImageInfo));
}

EGLHelper::~EGLHelper()
{

}

bool EGLHelper::SwapBuffers() {
	return eglSwapBuffers(m_EGLDisplay, m_EGLSurface);
}

void EGLHelper::SetPresentationTime(long nsecs) {

	if (!m_pPresentTimeFunc) {
		m_pPresentTimeFunc(m_EGLDisplay, m_EGLSurface, nsecs);
	}
}

int EGLHelper::Draw()
{

	LOGD("EGLHelper::Draw");

	if (!m_bEGLEnvReady)
		return 0;

	drawFBO ();

	++m_FrameID;
	return ERROR_OK;
}

int EGLHelper::SetRecordWindow(ANativeWindow *const pWindow)
{
	LOGD("EGLHelper::SetRecordWindow");
	if (nullptr == pWindow)
	{
		LOGE("SetRecordWindow pWindow is nullptr");
		return ERROR_INPUT;
	}
	m_pRecordWindow = pWindow;
	return 0;
}

int EGLHelper::CreateEGLEnv(int usage, ANativeWindow *const pWindow)
{
	LOGD ("EGLHelper::CreateEGLEnv");

	int retCode = ERROR_OK;
	do {
		// 1.获取EGLDisplay对象，与本地窗口系统建立连接
		m_EGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		if (EGL_NO_DISPLAY == m_EGLDisplay) {
			LOGE("EGLHelper::CreateEGLEnv eglGetDisplay failed");
			retCode = ERROR_EGL_STATE;
			break;
		}

		// 2.初始化EGL
		int nMajorVer = 0, nMinVer = 0;
		if (!eglInitialize(m_EGLDisplay, &nMajorVer, &nMinVer)) {
			LOGE("EGLHelper::CreateEGLEnv eglInitialize failed");
			retCode = ERROR_EGL_STATE;
			break;
		}
		LOGE("EGLHelper::CreateEGLEnv egl version nMajorVer = %d, nMinVer = %d", nMajorVer, nMinVer);

		// EGL config attributes
		const EGLint confAttr[] = {
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
				EGL_SURFACE_TYPE,
				EGL_PBUFFER_BIT,//EGL_WINDOW_BIT EGL_PBUFFER_BIT we will create a pixelbuffer surface
				EGL_RED_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_BLUE_SIZE, 8,
				EGL_ALPHA_SIZE, 8,// if you need the alpha channel
				EGL_DEPTH_SIZE, 8,// if you need the depth buffer
				EGL_STENCIL_SIZE, 8,
				EGL_NONE
		};

		// 3.获取EGLConfig对象
		EGLint numConfigs = 0;
		if (!eglChooseConfig(m_EGLDisplay, confAttr, &m_EGLConfig, 1, &numConfigs)) {
			LOGE("EGLHelper::CreateEGLEnv eglChooseConfig failed");
			retCode = ERROR_EGL_STATE;
			break;
		}

		// 4.创建渲染表面 EGLSurface，使用 elgCreatePbufferSurface 创建屏幕外渲染
		// surface attributes
		// the surface size is set to the input frame size
		const EGLint surfaceAttr[] = {
				EGL_WIDTH, 1,
				EGL_HEIGHT,1,
				EGL_NONE
		};

		// 这里也可以使用 eglCreateWindowSurface 传入一个 native window 用于绘制，这个 window 可以是java层
		// 的 GLSurfaceView，TextureView 或者 SurfaceView
		if (0 == usage) {
			m_EGLSurface = eglCreatePbufferSurface(m_EGLDisplay, m_EGLConfig, surfaceAttr);
		} else {
			if (nullptr != pWindow) {
				m_pRecordWindow = pWindow;
				m_EGLSurface = eglCreateWindowSurface(m_EGLDisplay, m_EGLConfig, m_pRecordWindow, nullptr);
			} else {
				LOGE("EGLHelper::CreateEGLEnv eglCreateWindowSurface pWindow is nullptr");
				retCode = ERROR_INPUT;
			}
		}
		if(m_EGLSurface == EGL_NO_SURFACE)
		{
			GLint error = eglGetError ();
			LOGE("EGLHelper::CreateEGLEnv eglCreateWindowSurface error = %d", error);
			switch(error)
			{
				case EGL_BAD_ALLOC:
					// Not enough resources available. Handle and recover
					LOGE("EGLHelper::CreateEGLEnv Not enough resources available");
					break;
				case EGL_BAD_CONFIG:
					// Verify that provided EGLConfig is valid
					LOGE("EGLHelper::CreateEGLEnv provided EGLConfig is invalid");
					break;
				case EGL_BAD_PARAMETER:
					// Verify that the EGL_WIDTH and EGL_HEIGHT are
					// non-negative values
					LOGE("EGLHelper::CreateEGLEnv provided EGL_WIDTH and EGL_HEIGHT is invalid");
					break;
				case EGL_BAD_MATCH:
					// Check window and EGLConfig attributes to determine
					// compatibility and pbuffer-texture parameters
					LOGE("EGLHelper::CreateEGLEnv Check window and EGLConfig attributes");
					break;
			}
			retCode = ERROR_EGL_STATE;
		}

		// 5.创建渲染上下文EGLContext
		// EGL context attributes
		const EGLint ctxAttr[] = {
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
		};
		m_EGLContext = eglCreateContext(m_EGLDisplay, m_EGLConfig, EGL_NO_CONTEXT, ctxAttr);
		if(m_EGLContext == EGL_NO_CONTEXT)
		{
			EGLint error = eglGetError();
			if(error == EGL_BAD_CONFIG)
			{
				// Handle error and recover
				LOGE("EGLHelper::CreateEGLEnv EGL_BAD_CONFIG");
				retCode = ERROR_EGL_STATE;
				break;
			}
		}

		// 6.绑定上下文
		if (!eglMakeCurrent(m_EGLDisplay, m_EGLSurfaceRender, m_EGLSurfaceRender, m_EGLContext)) {
			LOGE("EGLHelper::CreateEGLEnv eglMakeCurrent failed");
			retCode = ERROR_EGL_STATE;
			break;
		}

		m_pPresentTimeFunc = (PFNEGLPRESENTATIONTIMEANDROIDPROC)eglGetProcAddress("eglPresentationTimeANDROID");

		m_bEGLEnvReady = true;

	} while (false);

	return retCode;
}

int EGLHelper::DestroyEGLEnv()
{
	if (m_EGLDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(m_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(m_EGLDisplay, m_EGLContext);
		eglDestroySurface(m_EGLDisplay, m_EGLSurface);
		eglReleaseThread();
		eglTerminate(m_EGLDisplay);
	}
	m_EGLDisplay = EGL_NO_DISPLAY;
	m_EGLSurface = EGL_NO_SURFACE;
	m_EGLContext = EGL_NO_CONTEXT;

	return 0;
}

// todo: 目前强制传 rgb32 类型，后续更新
int EGLHelper::SetImageData(const int imgWidth, const int imgHeight, const unsigned char *pImgData)
{
	AUTO_COUNT_TIME_COST("EGLHelper::SetImageData")
	LOGD("EGLHelper::SetImageData imgWidth = %d, imgHeight = %d, pImgData = %p", imgWidth, imgHeight, pImgData);
	CHECK_NULL_INPUT(pImgData)

	int retCode = ERROR_OK;
	do {
		if (m_bEGLEnvReady) {
			retCode = ERROR_EGL_STATE;
			break;
		}

		m_RenderImg.width = imgWidth;
		m_RenderImg.height = imgHeight;
		m_RenderImg.wPitch[0] = m_RenderImg.width * 4;
		m_RenderImg.format = MY_FORMAT_RGB32;
		memcpy(m_RenderImg.ppBuffer[0], pImgData, m_RenderImg.width * m_RenderImg.height * 4);

	} while (false);

	return 0;
}

RESULT EGLHelper::createShader()
{
	LOGD("EGLHelper::createShader");
	m_pShaderHelperNormal = new ShaderHelper (triangle_vertex_shader0, triangle_fragment_shader0);
	RESULT ret = m_pShaderHelperNormal->getShaderHelperState();
	LOGD("createShader m_pShaderHelperNormal getShaderHelperState ret = %d", ret);
	CHECK_OK_RETURN(ret)

	m_pShaderHelperFBO = new ShaderHelper (triangle_vertex_shader0, triangle_fragment_shader1);
	ret = m_pShaderHelperFBO->getShaderHelperState();
	LOGD("createShader m_pShaderHelperFBO getShaderHelperState ret = %d", ret);
	CHECK_OK_RETURN(ret)

	return ret;
}

void EGLHelper::destroyShader()
{
	LOGD("EGLHelper::destroyShader");
	SafeDelete (m_pShaderHelperNormal);
	SafeDelete (m_pShaderHelperFBO);
}

RESULT EGLHelper::creteGLBuffer ()
{
	LOGD("EGLHelper::creteGLBuffer");

	int maxRenderBufferSize[4]{0};
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, maxRenderBufferSize);
	LOGD("creteGLBuffer maxRenderBufferSize (%d, %d, %d, %d)", maxRenderBufferSize[0], maxRenderBufferSize[1],
		 maxRenderBufferSize[2], maxRenderBufferSize[3]);


	glViewport(0, 0, m_WindowWidth, m_WindowHeight);
	int viewportSize[4]{0};
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	LOGD("creteGLBuffer viewportSize (%d, %d, %d, %d)", viewportSize[0], viewportSize[1],
		 viewportSize[2], viewportSize[3]);
	int screenWidth = viewportSize[2];
	int screenHeight = viewportSize[3];
	glGenFramebuffers(1, &m_FBO);
	GLenum targetFBO = GL_TEXTURE_2D;
	DrawHelper::GetOneTexture(targetFBO, &m_FBOTexture);
	glBindTexture(targetFBO, m_FBOTexture);
	glTexImage2D(targetFBO, GL_FALSE, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(targetFBO, GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, targetFBO, m_FBOTexture, 0);
	GLenum tmpStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != tmpStatus)
	{
		LOGE("creteGLBuffer GL_FRAMEBUFFER error");
		return ERROR_GL_STATE;
	}

	std::vector <float> vertex_multi{
			0.3199f, -0.5f, 0,
			-0.3199f, 0.5f, 0,
			0.3199f, 0.5f, 0,
			-0.3199f, -0.5f, 0
	};
	std::vector<int> index_multi{
			0, 1, 3,
			0, 2, 3
	};

	glGenVertexArrays(1, &m_VAO);
	DrawHelper::CheckGLError("creteGLBuffer glGenBuffers");
	glGenBuffers(1, &m_VBO);
	DrawHelper::CheckGLError("creteGLBuffer glGenBuffers");
	glGenBuffers(1, &m_EBO);
	DrawHelper::CheckGLError("creteGLBuffer glGenBuffers");

	bool bStatic = true;
	long lSize = 0;

	if (bStatic)
	{
		lSize = sizeof(float) * vertex_multi.size();
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, lSize, &vertex_multi[0], GL_STATIC_DRAW);
		DrawHelper::CheckGLError("creteGLBuffer glBufferData");

		lSize = sizeof(int) * index_multi.size();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, lSize, &index_multi[0], GL_STATIC_DRAW);
		DrawHelper::CheckGLError("creteGLBuffer glBufferData");
	}
	else
	{
		/*lSize = sizeof(float) * m_vertices.size();
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, lSize, &m_vertices[0], GL_STATIC_DRAW);
		LOGD("creteGLBuffer %p %p %p %p %p %p", &m_vertices[0], &m_vertices[1], &m_vertices[2],
			 &m_vertices[3], &m_vertices[4], &m_vertices[5]);
		DrawHelper::CheckGLError("creteGLBuffer glBufferData");

		lSize = sizeof(int) * m_Indices.size();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, lSize, &m_Indices[0], GL_STATIC_DRAW);
		DrawHelper::CheckGLError("creteGLBuffer glBufferData");*/
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	DrawHelper::CheckGLError("creteGLBuffer glVertexAttribPointer");

	glBindVertexArray(GL_NONE);
	DrawHelper::CheckGLError("creteGLBuffer glBindVertexArray");

	return ERROR_OK;
}

void EGLHelper::destroyGLBuffer ()
{
	LOGD("EGLHelper::DestroyGLBuffer");
	SafeDeleteGLBuffer (1, &m_VAO);
	SafeDeleteGLBuffer (1, &m_VBO);
	SafeDeleteGLBuffer (1, &m_EBO);
	m_VAO = m_VBO = m_EBO = GL_NONE;
}

int EGLHelper::SetWindow(ANativeWindow *const pNativeWindow, const int windowWith, const int windowHeight)
{
	LOGD("SetWindow windowWith = %d, windowHeight = %d", windowWith, windowHeight);
	m_pANativeWindow = pNativeWindow;
	m_VideoWidth = windowWith;
	m_VideoHeight = windowHeight;
	return 0;
}

int EGLHelper::SetWindowRender(ANativeWindow *const pNativeWindow, const int windowWith, const int windowHeight)
{
	LOGD("SetWindowRender windowWith = %d, windowHeight = %d", windowWith, windowHeight);
	m_pANativeWindowRender = pNativeWindow;
	m_WindowWidth = windowWith;
	m_WindowHeight = windowHeight;
	return 0;
}

int EGLHelper::drawFBO()
{
	LOGD("EGLHelper::drawFBO");

	if (0 == m_lBeginTime) {
		m_lBeginTime = MyTimeUtils::GetCurrentTime();
	}

	float rotateAngle = (MyTimeUtils::GetCurrentTime() - m_lBeginTime) * 0.5f;
	glm::mat4 mvp(1.f);
	mvp = glm::rotate(glm::mat4(1.f), glm::radians(rotateAngle), glm::vec3(0, 0, 1.f));

	// draw FBO
	m_pShaderHelperNormal->use();
	m_pShaderHelperNormal->setMat4("mvp", mvp);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(GL_NONE);
	DrawHelper::CheckGLError("OnDrawFrame glBindVertexArray");

	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// draw FBO texture to screen
	m_pShaderHelperFBO->use();
	m_pShaderHelperFBO->setMat4("mvp", glm::mat4(mvp));

	glClear (GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, m_FBOTexture);
	DrawHelper::CheckGLError("OnDrawFrame glBindTexture");
	glBindVertexArray(m_VAO);
	DrawHelper::CheckGLError("OnDrawFrame glBindVertexArray");
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
	DrawHelper::CheckGLError("OnDrawFrame glDrawElements");
	glBindVertexArray(GL_NONE);
	DrawHelper::CheckGLError("OnDrawFrame glBindVertexArray");

	return ERROR_OK;
}
