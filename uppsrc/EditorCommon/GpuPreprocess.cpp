#include <EditorCommon/EditorCommon.h>
#include <EditorCommon/GpuPreprocess.h>

#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#ifdef Display
#undef Display
#endif
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <drm/drm_fourcc.h>

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLDELETEPROGRAMPROC glDeleteProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLUNIFORM1IPROC glUniform1i;
static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif
#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif

typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);

static PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

// EGL extension function pointers for DMA-BUF import (Phase 12)
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATEIMAGEKHRPROC)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLDESTROYIMAGEKHRPROC)(EGLDisplay dpy, EGLImageKHR image);
typedef void (APIENTRY *PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)(GLenum target, GLeglImageOES image);

static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

// EGL_LINUX_DMA_BUF_EXT constants
#ifndef EGL_LINUX_DMA_BUF_EXT
#define EGL_LINUX_DMA_BUF_EXT 0x3270
#endif
#ifndef EGL_LINUX_DRM_FOURCC_EXT
#define EGL_LINUX_DRM_FOURCC_EXT 0x3271
#endif
#ifndef EGL_DMA_BUF_PLANE0_FD_EXT
#define EGL_DMA_BUF_PLANE0_FD_EXT 0x3272
#endif
#ifndef EGL_DMA_BUF_PLANE0_OFFSET_EXT
#define EGL_DMA_BUF_PLANE0_OFFSET_EXT 0x3273
#endif
#ifndef EGL_DMA_BUF_PLANE0_PITCH_EXT
#define EGL_DMA_BUF_PLANE0_PITCH_EXT 0x3274
#endif

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

static void* GetAnyGLFunc(const char* name) {
	typedef void* (*PFNGLXGETPROCADDRESSARB)(const GLubyte*);
	static PFNGLXGETPROCADDRESSARB glx_gpa = (PFNGLXGETPROCADDRESSARB)dlsym(RTLD_DEFAULT, "glXGetProcAddressARB");
	if (!glx_gpa) glx_gpa = (PFNGLXGETPROCADDRESSARB)dlsym(RTLD_DEFAULT, "glXGetProcAddress");

	void* p = nullptr;
	if (glx_gpa) p = glx_gpa((const GLubyte*)name);
	if (p) return p;

	typedef void* (*PFNEGLGETPROCADDRESS)(const char*);
	static PFNEGLGETPROCADDRESS egl_gpa = (PFNEGLGETPROCADDRESS)dlsym(RTLD_DEFAULT, "eglGetProcAddress");
	if (egl_gpa) {
		p = egl_gpa(name);
		if (p) return p;
	}

	return dlsym(RTLD_DEFAULT, name);
}

typedef const GLubyte * (GLAPIENTRY * PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
static PFNGLGETSTRINGIPROC glGetStringi;

static void LoadGLExtensions() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;
	glGetStringi = (PFNGLGETSTRINGIPROC)GetAnyGLFunc("glGetStringi");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GetAnyGLFunc("glGenFramebuffers");
	if (!glGenFramebuffers) glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GetAnyGLFunc("glGenFramebuffersEXT");
	
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GetAnyGLFunc("glDeleteFramebuffers");
	if (!glDeleteFramebuffers) glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GetAnyGLFunc("glDeleteFramebuffersEXT");
	
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetAnyGLFunc("glBindFramebuffer");
	if (!glBindFramebuffer) glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetAnyGLFunc("glBindFramebufferEXT");
	
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetAnyGLFunc("glFramebufferTexture2D");
	if (!glFramebufferTexture2D) glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetAnyGLFunc("glFramebufferTexture2DEXT");
	
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetAnyGLFunc("glCheckFramebufferStatus");
	if (!glCheckFramebufferStatus) glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetAnyGLFunc("glCheckFramebufferStatusEXT");

	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetAnyGLFunc("glBlitFramebuffer");
	if (!glBlitFramebuffer) glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetAnyGLFunc("glBlitFramebufferEXT");

	glCreateShader = (PFNGLCREATESHADERPROC)GetAnyGLFunc("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)GetAnyGLFunc("glDeleteShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GetAnyGLFunc("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GetAnyGLFunc("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)GetAnyGLFunc("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GetAnyGLFunc("glGetShaderInfoLog");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetAnyGLFunc("glCreateProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetAnyGLFunc("glDeleteProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)GetAnyGLFunc("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)GetAnyGLFunc("glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetAnyGLFunc("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetAnyGLFunc("glGetProgramInfoLog");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GetAnyGLFunc("glUseProgram");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)GetAnyGLFunc("glGetAttribLocation");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GetAnyGLFunc("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)GetAnyGLFunc("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FPROC)GetAnyGLFunc("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)GetAnyGLFunc("glUniform2f");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetAnyGLFunc("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)GetAnyGLFunc("glDisableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GetAnyGLFunc("glVertexAttribPointer");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetAnyGLFunc("glBlitFramebuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetAnyGLFunc("glCheckFramebufferStatus");
	glGenBuffers = (PFNGLGENBUFFERSPROC)GetAnyGLFunc("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)GetAnyGLFunc("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)GetAnyGLFunc("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)GetAnyGLFunc("glDeleteBuffers");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GetAnyGLFunc("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetAnyGLFunc("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)GetAnyGLFunc("glDeleteVertexArrays");

	// Load EGL extension functions for DMA-BUF import (Phase 12)
	typedef void* (*PFNEGLGETPROCADDRESS)(const char*);
	PFNEGLGETPROCADDRESS get_egl_proc = (PFNEGLGETPROCADDRESS)GetAnyGLFunc("eglGetProcAddress");
	if (get_egl_proc) {
		eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)get_egl_proc("eglCreateImageKHR");
		eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)get_egl_proc("eglDestroyImageKHR");
	}
	glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)GetAnyGLFunc("glEGLImageTargetTexture2DOES");

	loaded = true;
}

#endif

static const float g_brief_pattern[] = {
    8, -3, 9, 5,
    4, 2, 7, -12,
    -11, 9, -8, 2,
    7, -12, 12, -13,
    2, -13, 2, 12,
    1, -7, 1, 6,
    -2, -10, -2, -4,
    -13, -13, -11, -8,
    -13, -3, -12, -9,
    10, 4, 11, 9,
    -13, -8, -8, -9,
    -11, 7, -9, 12,
    7, 7, 12, 6,
    -4, -5, -3, 0,
    -13, 2, -12, -3,
    -9, 0, -7, 5,
    12, -6, 12, -1,
    -3, 6, -2, 12,
    -6, -13, -4, -8,
    11, -13, 12, -8,
    4, 7, 5, 1,
    5, -3, 10, -3,
    3, -7, 6, 12,
    -8, -7, -6, -2,
    -2, 11, -1, -10,
    -13, 12, -8, 10,
    -7, 3, -5, -3,
    -4, 2, -3, 7,
    -10, -12, -6, 11,
    5, -12, 6, -7,
    5, -6, 7, -1,
    1, 0, 4, -5,
    9, 11, 11, -13,
    4, 7, 4, 12,
    2, -1, 4, 4,
    -4, -12, -2, 7,
    -8, -5, -7, -10,
    4, 11, 9, 12,
    0, -8, 1, -13,
    -13, -2, -8, 2,
    -3, -2, -2, 3,
    -6, 9, -4, -9,
    8, 12, 10, 7,
    0, 9, 1, 3,
    7, -5, 11, -10,
    -13, -6, -11, 0,
    10, 7, 12, 1,
    -6, -3, -6, 12,
    10, -9, 12, -4,
    -13, 8, -8, -12,
    -13, 0, -8, -4,
    3, 3, 7, 8,
    5, 7, 10, -7,
    -1, 7, 1, -12,
    3, -10, 5, 6,
    2, -4, 3, -10,
    -13, 0, -13, 5,
    -13, -7, -12, 12,
    -13, 3, -11, 8,
    -7, 12, -4, 7,
    6, -10, 12, 8,
    -9, -1, -7, -6,
    -2, -5, 0, 12,
    -12, 5, -7, 5,
    3, -10, 8, -13,
    -7, -7, -4, 5,
    -3, -2, -1, -7,
    2, 9, 5, -11,
    -11, -13, -5, -13,
    -1, 6, 0, -1,
    5, -3, 5, 2,
    -4, -13, -4, 12,
    -9, -6, -9, 6,
    -12, -10, -8, -4,
    10, 2, 12, -3,
    7, 12, 12, 12,
    -7, -13, -6, 5,
    -4, 9, -3, 4,
    7, -1, 12, 2,
    -7, 6, -5, 1,
    -13, 11, -12, 5,
    -3, 7, -2, -6,
    7, -8, 12, -7,
    -13, -7, -11, -12,
    1, -3, 12, 12,
    2, -6, 3, 0,
    -4, 3, -2, -13,
    -1, -13, 1, 9,
    7, 1, 8, -6,
    1, -1, 3, 12,
    9, 1, 12, 6,
    -1, -9, -1, 3,
    -13, -13, -10, 5,
    7, 7, 10, 12,
    12, -5, 12, 9,
    6, 3, 7, 11,
    5, -13, 6, 10,
    2, -12, 2, 3,
    3, 8, 4, -6,
    2, 6, 12, -13,
    9, -12, 10, 3,
    -8, 4, -7, 9,
    -11, 12, -4, -6,
    1, 12, 2, -8,
    6, -9, 7, -4,
    2, 3, 3, -2,
    6, 3, 11, 0,
    3, -3, 8, -8,
    7, 8, 9, 3,
    -11, -5, -6, -4,
    -10, 11, -5, 10,
    -5, -8, -3, 12,
    -10, 5, -9, 0,
    8, -1, 12, -6,
    4, -6, 6, -11,
    -10, 12, -8, 7,
    4, -2, 6, 7,
    -2, 0, -2, 12,
    -5, -8, -5, 2,
    7, -6, 10, 12,
    -9, -13, -8, -8,
    -5, -13, -5, -2,
    8, -8, 9, -13,
    -9, -11, -9, 0,
    1, -8, 1, -2,
    7, -4, 9, 1,
    -2, 1, -1, -4,
    11, -6, 12, -11,
    -12, -9, -6, 4,
    3, 7, 7, 12,
    5, 5, 10, 8,
    0, -4, 2, 8,
    -9, 12, -5, -13,
    0, 7, 2, 12,
    -1, 2, 1, 7,
    5, 11, 7, -9,
    3, 5, 6, -8,
    -13, -4, -8, 9,
    -5, 9, -3, -3,
    -4, -7, -3, -12,
    6, 5, 8, 0,
    -7, 6, -6, 12,
    -13, 6, -5, -2,
    1, -10, 3, 10,
    4, 1, 8, -4,
    -2, -2, 2, -13,
    2, -12, 12, 12,
    -2, -13, 0, -6,
    4, 1, 9, 3,
    -6, -10, -3, -5,
    -3, -13, -1, 1,
    7, 5, 12, -11,
    4, -2, 5, -7,
    -13, 9, -9, -5,
    7, 1, 8, 6,
    7, -8, 7, 6,
    -7, -4, -7, 1,
    -8, 11, -7, -8,
    -13, 6, -12, -8,
    2, 4, 3, 9,
    10, -5, 12, 3,
    -6, -5, -6, 7,
    8, -3, 9, -8,
    2, -12, 2, 8,
    -11, -2, -10, 3,
    -12, -13, -7, -9,
    -11, 0, -10, -5,
    5, -3, 11, 8,
    -2, -13, -1, 12,
    -1, -8, 0, 9,
    -13, -11, -12, -5,
    -10, -2, -10, 11,
    -3, 9, -2, -13,
    2, -3, 3, 2,
    -9, -13, -4, 0,
    -4, 6, -3, -10,
    -4, 12, -2, -7,
    -6, -11, -4, 9,
    6, -3, 6, 11,
    -13, 11, -5, 5,
    11, 11, 12, 6,
    7, -5, 12, -2,
    -1, 12, 0, 7,
    -4, -8, -3, -2,
    -7, 1, -6, 7,
    -13, -12, -8, -13,
    -7, -2, -6, -8,
    -8, 5, -6, -9,
    -5, -1, -4, 5,
    -13, 7, -8, 10,
    1, 5, 5, -13,
    1, 0, 10, -13,
    9, 12, 10, -1,
    5, -8, 10, -9,
    -1, 11, 1, -13,
    -9, -3, -6, 2,
    -1, -10, 1, 12,
    -13, 1, -8, -10,
    8, -11, 10, -6,
    2, -13, 3, -6,
    7, -13, 12, -9,
    -10, -10, -5, -7,
    -10, -8, -8, -13,
    4, -6, 8, 5,
    3, 12, 8, -13,
    -4, 2, -3, -3,
    5, -13, 10, -12,
    4, -13, 5, -1,
    -9, 9, -4, 3,
    0, 3, 3, -9,
    -12, 1, -6, 1,
    3, 2, 4, -8,
    -10, -10, -10, 9,
    8, -13, 12, 12,
    -8, -12, -6, -5,
    2, 2, 3, 7,
    10, 6, 11, -8,
    6, 8, 8, -12,
    -7, 10, -6, 5,
    -3, -9, -3, 9,
    -1, -13, -1, 5,
    -3, -7, -3, 4,
    -8, -2, -8, 3,
    4, 2, 12, 12,
    2, -5, 3, 11,
    6, -9, 11, -13,
    3, -1, 7, 12,
    11, -1, 12, 4,
    -3, 0, -3, 6,
    4, -11, 4, 12,
    2, -4, 2, 1,
    -10, -6, -8, 1,
    -13, 7, -11, 1,
    -13, 12, -11, -13,
    6, 0, 11, -13,
    0, -1, 1, 4,
    -13, 3, -9, -2,
    -9, 8, -6, -3,
    -13, -6, -8, -2,
    5, -9, 8, 10,
    2, 7, 3, -9,
    -1, -6, -1, -1,
    9, 5, 11, -2,
    11, -3, 12, -8,
    3, 0, 3, 5,
    -1, 4, 0, 10,
    3, -6, 4, 5,
    -13, 0, -10, 5,
    5, 8, 12, 11,
    8, 9, 9, -6,
    7, -4, 8, -12,
    -10, 4, -10, 9,
    7, 3, 12, 4,
    9, -7, 10, -2,
    7, 0, 12, -2,
    -1, -6, 0, -11,
};

namespace Upp {

#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)

static GpuFrame::ExternalFormat InferExternalFormatFromDmaBuf(uint32_t fourcc) {
	switch (fourcc) {
	case DRM_FORMAT_ARGB8888:
		return GpuFrame::EXTERNAL_RGBA8;
#ifdef DRM_FORMAT_ABGR8888
	case DRM_FORMAT_ABGR8888:
		return GpuFrame::EXTERNAL_BGRA8;
#endif
#ifdef DRM_FORMAT_XRGB8888
	case DRM_FORMAT_XRGB8888:
		return GpuFrame::EXTERNAL_RGBA8;
#endif
#ifdef DRM_FORMAT_XBGR8888
	case DRM_FORMAT_XBGR8888:
		return GpuFrame::EXTERNAL_BGRA8;
#endif
#ifdef DRM_FORMAT_YUYV
	case DRM_FORMAT_YUYV:
		return GpuFrame::EXTERNAL_YUYV422;
#endif
	default:
		return GpuFrame::EXTERNAL_UNKNOWN;
	}
}

static const char* GetExternalFormatName(GpuFrame::ExternalFormat fmt) {
	switch (fmt) {
	case GpuFrame::EXTERNAL_RGBA8: return "RGBA8";
	case GpuFrame::EXTERNAL_BGRA8: return "BGRA8";
	case GpuFrame::EXTERNAL_YUYV422: return "YUYV422";
	default: return "UNKNOWN";
	}
}

static bool IsDirectRgbaLikeExternalFormat(GpuFrame::ExternalFormat fmt) {
	return fmt == GpuFrame::EXTERNAL_RGBA8 || fmt == GpuFrame::EXTERNAL_BGRA8;
}

static byte ClampToByte(int v) {
	return (byte)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

static void ConvertYuyv422RowToRgba(byte* dst, const byte* src, int width) {
	for (int x = 0; x < width; x += 2) {
		int y0 = src[0];
		int u  = src[1] - 128;
		int y1 = src[2];
		int v  = src[3] - 128;
		int c0 = y0 - 16;
		int c1 = y1 - 16;
		int d = u;
		int e = v;
		int r0 = (298 * c0 + 409 * e + 128) >> 8;
		int g0 = (298 * c0 - 100 * d - 208 * e + 128) >> 8;
		int b0 = (298 * c0 + 516 * d + 128) >> 8;
		int r1 = (298 * c1 + 409 * e + 128) >> 8;
		int g1 = (298 * c1 - 100 * d - 208 * e + 128) >> 8;
		int b1 = (298 * c1 + 516 * d + 128) >> 8;
		dst[0] = ClampToByte(r0);
		dst[1] = ClampToByte(g0);
		dst[2] = ClampToByte(b0);
		dst[3] = 255;
		dst[4] = ClampToByte(r1);
		dst[5] = ClampToByte(g1);
		dst[6] = ClampToByte(b1);
		dst[7] = 255;
		src += 4;
		dst += 8;
	}
}

static bool ConvertMappedYuyv422ToRgba(Vector<byte>& rgba, const void* mapped, int width, int height, int stride) {
	if (!mapped || width <= 0 || height <= 0)
		return false;
	int src_stride = stride > 0 ? stride : width * 2;
	rgba.SetCount(width * height * 4);
	const byte* src = (const byte*)mapped;
	byte* dst = rgba.Begin();
	for (int y = 0; y < height; y++) {
		ConvertYuyv422RowToRgba(dst + y * width * 4, src + y * src_stride, width);
	}
	return true;
}

// Phase 12 Stage 1: EGLImage DMA-BUF import helpers
static EGLImageKHR ImportDmaBufAsEGLImage(EGLDisplay dpy, int dma_fd,
                                          int width, int height, uint32_t fourcc,
                                          int stride, uint64_t modifier) {
	EGLint pitch = stride > 0 ? stride : width * 4;
	if (stride <= 0) {
		if (fourcc == DRM_FORMAT_YUYV)
			pitch = width * 2;
#ifdef DRM_FORMAT_UYVY
		else if (fourcc == DRM_FORMAT_UYVY)
			pitch = width * 2;
#endif
	}
	EGLint attribs[32];
	int i = 0;
	attribs[i++] = EGL_WIDTH;
	attribs[i++] = width;
	attribs[i++] = EGL_HEIGHT;
	attribs[i++] = height;
	attribs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[i++] = (EGLint)fourcc;
	attribs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
	attribs[i++] = dma_fd;
	attribs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
	attribs[i++] = 0;
	attribs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
	attribs[i++] = pitch;
	if (modifier != 0) {
		attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
		attribs[i++] = (EGLint)(modifier & 0xffffffffu);
		attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
		attribs[i++] = (EGLint)((modifier >> 32) & 0xffffffffu);
	}
	attribs[i++] = EGL_NONE;
	if (!eglCreateImageKHR) return EGL_NO_IMAGE_KHR;
	return eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
}

static bool BindEGLImageToTexture(GLuint tex, EGLImageKHR image, GLenum target = GL_TEXTURE_2D) {
	if (!glEGLImageTargetTexture2DOES) return false;
	glBindTexture(target, tex);
	glEGLImageTargetTexture2DOES(target, image);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLenum err = glGetError();
	return (err == GL_NO_ERROR);
}


static void DestroyEglImage(EGLDisplay dpy, EGLImageKHR image) {
	if (eglDestroyImageKHR && image != EGL_NO_IMAGE_KHR)
		eglDestroyImageKHR(dpy, image);
}

class GpuPreprocessBackend {
public:
	virtual ~GpuPreprocessBackend() {}
	virtual bool Initialize(const GpuPreprocessConfig& cfg, String& error) = 0;
	virtual void Shutdown() = 0;
	virtual bool IsReady() const = 0;
	virtual String GetRoute() const = 0;
	virtual String GetRenderer() const = 0;
	virtual bool PrepareFrameGpu(const Image& img, int pyramid_levels, GpuPreprocessStats& stats, String& error) = 0;
	virtual bool PrepareFrameDmaBuf(const GpuFrame::DmaBufDesc& dmabuf, int pyramid_levels, GpuPreprocessStats& stats, String& error) = 0;  // Phase 12 Stage 3 (Linux)
	virtual bool PrepareFrameD3D11(const GpuFrame::D3D11TextureDesc& d3d11, int pyramid_levels, GpuPreprocessStats& stats, String& error) = 0; // Phase 12 Stage 4 (Windows)
	virtual bool ComputeScoreMapsGpu(GpuPreprocessStats& stats, String& error) = 0;
	virtual bool GetKeypointsGpu(Vector<GpuKp>& out, int max_keypoints, String& error) = 0;
	virtual bool ComputeDescriptorsGpu(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors, String& error) = 0;
	virtual bool ExtractSparsePatchesGpu(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches, String& error) = 0;
	virtual bool ReadbackAreasGpu(const Vector<Rect>& rects, Vector<ByteMat>& outcomes, String& error) = 0;
	virtual bool ReadbackBinarizedAreasGpu(const Vector<Rect>& rects, float threshold, Vector<ByteMat>& outcomes, String& error) = 0;
	virtual bool GetGrayscaleGpu(ByteMat& out, String& error) = 0;
	virtual bool GetSmoothGpu(ByteMat& out, String& error) = 0;
	virtual bool GetPyramidGrayGpu(Vector<ByteMat>& out, String& error) = 0;
	virtual bool GetPyramidSmoothGpu(Vector<ByteMat>& out, String& error) = 0;
	virtual void MakeCurrent() = 0;
};

static GLuint CompileShader(GLenum type, const char* source, String& error) {
	GLuint shader = glCreateShader(type);
	if (!shader) {
		error = "glCreateShader failed";
		return 0;
	}
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		error = String("Shader compile error: ") + log;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint LinkProgram(GLuint vs, GLuint fs, String& error) {
	GLuint prog = glCreateProgram();
	if (!prog) {
		error = "glCreateProgram failed";
		return 0;
	}
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	GLint ok = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
		error = String("Program link error: ") + log;
		glDeleteProgram(prog);
		return 0;
	}
	return prog;
}

struct EglProbeResult {
	bool ok = false;
	String route;
	String renderer;
	String error;
};

static bool ChooseProbeConfig(EGLDisplay dpy, EGLConfig& cfg) {
	const EGLint attrs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_NONE
	};
	EGLint count = 0;
	return eglChooseConfig(dpy, attrs, &cfg, 1, &count) == EGL_TRUE && count > 0;
}

static bool MakeProbeContext(EGLDisplay dpy, EGLConfig cfg, String& renderer, String& error) {
	if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
		error = "eglBindAPI failed";
		return false;
	}
	const EGLint ctx_attrs[] = {
		0x3098, 3, // EGL_CONTEXT_MAJOR_VERSION
		0x30FB, 3, // EGL_CONTEXT_MINOR_VERSION
		0x30FD, 0x00000001, // EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
		EGL_NONE
	};
	EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attrs);
	if (ctx == EGL_NO_CONTEXT) {
		ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, nullptr); // Fallback
	}
	if (ctx == EGL_NO_CONTEXT) {
		error = "eglCreateContext failed";
		return false;
	}
	const EGLint pbuf_attrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
	EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbuf_attrs);
	if (surf == EGL_NO_SURFACE) {
		eglDestroyContext(dpy, ctx);
		error = "eglCreatePbufferSurface failed";
		return false;
	}
	bool ok = eglMakeCurrent(dpy, surf, surf, ctx) == EGL_TRUE;
	if (ok) {
		const GLubyte* s = glGetString(GL_RENDERER);
		if (s)
			renderer = (const char*)s;
	}
	else
		error = "eglMakeCurrent failed";
	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface(dpy, surf);
	eglDestroyContext(dpy, ctx);
	return ok;
}

static EglProbeResult ProbeEglSurfaceless();
static EglProbeResult ProbeEglX11();

class GlBackendBase : public GpuPreprocessBackend {
protected:
	bool ready = false;
	String renderer;

	GLuint tex_rgba = 0;
	GLuint tex_gray = 0;
	GLuint tex_smooth = 0;
	Vector<GLuint> tex_pyramid_gray;
	Vector<GLuint> tex_pyramid_smooth;
	Vector<GLuint> tex_scoremap;
	Vector<GLuint> tex_nms;
	GLuint fbo = 0;
	GLuint prog_grayscale = 0;
	GLuint prog_grayscale_external = 0;
	GLuint prog_rgba = 0;
	GLuint prog_yuyv_manual = 0;
	GLenum upload_format = GL_RGBA;
	GLuint prog_blur_h = 0;
	GLuint prog_blur_v = 0;
	GLuint prog_downsample = 0;
	GLuint prog_fast9 = 0;
	GLuint prog_nms = 0;
	GLuint prog_describe = 0;
	GLuint prog_binarize = 0;
	int tex_width = 0;
	int tex_height = 0;
	int tex_rgba_width = 0;
	int pyramid_levels = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint tex_pattern = 0;
	GLuint tex_descriptors = 0;
	GLuint tex_kps = 0;
	GLuint tex_batch = 0;

public:
	virtual ~GlBackendBase() {}

	virtual bool IsReady() const override { return ready; }
	virtual String GetRenderer() const override { return renderer; }

	bool ProcessPreparedGrayscaleTexture(int w, int h, int levels, GpuPreprocessStats& stats) {
		TimeStop blur_ts;
		BlurTexture(tex_gray, tex_smooth, w, h);
		stats.blur_us = blur_ts.Elapsed();

		TimeStop pyr_ts;
		int pw = w, ph = h;
		for (int i = 0; i < levels; i++) {
			if (i == 0) {
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gray, 0);
				glBindTexture(GL_TEXTURE_2D, tex_pyramid_gray[i]);
				glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, pw, ph);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				BlurTexture(tex_pyramid_gray[i], tex_pyramid_smooth[i], pw, ph);
			} else {
				int prev_w = pw * 2;
				int prev_h = ph * 2;
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_pyramid_gray[i], 0);
				glViewport(0, 0, pw, ph);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex_pyramid_gray[i - 1]);
				glUseProgram(prog_downsample);
				glUniform1i(glGetUniformLocation(prog_downsample, "u_tex"), 0);
				glUniform2f(glGetUniformLocation(prog_downsample, "u_texel"), 1.0f / prev_w, 1.0f / prev_h);
				RenderFullscreenQuad(prog_downsample);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				BlurTexture(tex_pyramid_gray[i], tex_pyramid_smooth[i], pw, ph);
			}
			if (pw <= 1 && ph <= 1) break;
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		stats.pyramid_levels = levels;
		stats.pyramid_us = pyr_ts.Elapsed();
		return true;
	}

	bool ProcessPreparedRgbaTexture(int w, int h, int levels, GpuPreprocessStats& stats) {
		TimeStop gray_ts;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gray, 0);
		glViewport(0, 0, w, h);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_rgba);
		glUseProgram(prog_grayscale);
		glUniform1i(glGetUniformLocation(prog_grayscale, "u_tex"), 0);
		RenderFullscreenQuad(prog_grayscale);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		stats.grayscale_us = gray_ts.Elapsed();
		return ProcessPreparedGrayscaleTexture(w, h, levels, stats);
	}

	bool ProcessPreparedExternalTexture(int w, int h, int levels, GpuPreprocessStats& stats) {
		if (!prog_grayscale_external) return false;
		TimeStop gray_ts;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gray, 0);
		glViewport(0, 0, w, h);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex_rgba);
		glUseProgram(prog_grayscale_external);
		glUniform1i(glGetUniformLocation(prog_grayscale_external, "u_tex"), 0);
		RenderFullscreenQuad(prog_grayscale_external);
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		stats.grayscale_us = gray_ts.Elapsed();
		return ProcessPreparedGrayscaleTexture(w, h, levels, stats);
	}

	bool ProcessPreparedYuyvManualTexture(int w, int h, int levels, GpuPreprocessStats& stats) {
		if (!prog_yuyv_manual) return false;
		TimeStop gray_ts;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gray, 0);
		glViewport(0, 0, w, h);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_rgba);
		glUseProgram(prog_yuyv_manual);
		glUniform1i(glGetUniformLocation(prog_yuyv_manual, "u_tex"), 0);
		glUniform2f(glGetUniformLocation(prog_yuyv_manual, "u_texel"), 1.0f / w, 1.0f / h);
		RenderFullscreenQuad(prog_yuyv_manual);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		stats.grayscale_us = gray_ts.Elapsed();
		return ProcessPreparedGrayscaleTexture(w, h, levels, stats);
	}

	bool HasExtension(const char* ext) {
		GLint n = 0;
		if (glGetIntegerv) glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		if (n > 0 && glGetStringi) {
			for (int i = 0; i < n; i++) {
				const char* s = (const char*)glGetStringi(GL_EXTENSIONS, i);
				if (s && strcmp(s, ext) == 0) return true;
			}
		} else {
			const char* s = (const char*)glGetString(GL_EXTENSIONS);
			if (!s) return false;
			const char* p = strstr(s, ext);
			if (!p) return false;
			char end = p[strlen(ext)];
			if (end == ' ' || end == '\0') return true;
		}
		return false;
	}

	bool InitShaders(String& error) {
		const char* vs_passthrough = R"(
			#version 120
			attribute vec2 pos;
			attribute vec2 uv;
			varying vec2 v_uv;
			void main() {
				v_uv = uv;
				gl_Position = vec4(pos, 0.0, 1.0);
			}
		)";

		const char* fs_grayscale = R"(
			#version 120
			uniform sampler2D u_tex;
			varying vec2 v_uv;
			void main() {
				vec4 rgba = texture2D(u_tex, v_uv);
				float gray = 0.30078125 * rgba.r + 0.58984375 * rgba.g + 0.109375 * rgba.b;
				gl_FragColor = vec4(gray, gray, gray, 1.0);
			}
		)";

		const char* fs_grayscale_external = R"(
			#version 120
			#extension GL_OES_EGL_image_external : require
			uniform samplerExternalOES u_tex;
			varying vec2 v_uv;
			void main() {
				vec4 rgba = texture2D(u_tex, v_uv);
				float gray = 0.30078125 * rgba.r + 0.58984375 * rgba.g + 0.109375 * rgba.b;
				gl_FragColor = vec4(gray, gray, gray, 1.0);
			}
		)";

		const char* fs_yuyv_manual = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			varying vec2 v_uv;
			void main() {
				vec4 p = texture2D(u_tex, v_uv);
				float x_pix = v_uv.x / u_texel.x;
				float gray = (mod(x_pix, 2.0) < 1.0) ? p.r : p.b;
				gl_FragColor = vec4(gray, 0.0, 0.0, 1.0);
			}
		)";

		const char* fs_blur_h = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			varying vec2 v_uv;
			const float weights[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
			void main() {
				float result = texture2D(u_tex, v_uv).r * weights[0];
				for(int i = 1; i < 5; i++) {
					result += texture2D(u_tex, v_uv + vec2(u_texel.x * float(i), 0.0)).r * weights[i];
					result += texture2D(u_tex, v_uv - vec2(u_texel.x * float(i), 0.0)).r * weights[i];
				}
				gl_FragColor = vec4(result, 0.0, 0.0, 1.0);
			}
		)";

		const char* fs_blur_v = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			varying vec2 v_uv;
			const float weights[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
			void main() {
				float result = texture2D(u_tex, v_uv).r * weights[0];
				for(int i = 1; i < 5; i++) {
					result += texture2D(u_tex, v_uv + vec2(0.0, u_texel.y * float(i))).r * weights[i];
					result += texture2D(u_tex, v_uv - vec2(0.0, u_texel.y * float(i))).r * weights[i];
				}
				gl_FragColor = vec4(result, 0.0, 0.0, 1.0);
			}
		)";

		const char* fs_downsample = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			varying vec2 v_uv;
			void main() {
				vec2 uv0 = v_uv;
				vec2 uv1 = v_uv + vec2(u_texel.x, 0.0);
				vec2 uv2 = v_uv + vec2(0.0, u_texel.y);
				vec2 uv3 = v_uv + vec2(u_texel.x, u_texel.y);
				float s0 = texture2D(u_tex, uv0).r;
				float s1 = texture2D(u_tex, uv1).r;
				float s2 = texture2D(u_tex, uv2).r;
				float s3 = texture2D(u_tex, uv3).r;
				float result = (s0 + s1 + s2 + s3) * 0.25;
				gl_FragColor = vec4(result, 0.0, 0.0, 1.0);
			}
		)";

		const char* fs_fast9 = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			uniform float u_threshold;
			varying vec2 v_uv;
			void main() {
				float center = texture2D(u_tex, v_uv).r;
				vec2 offsets[16];
				offsets[0] = vec2(0, -3); offsets[1] = vec2(1, -3); offsets[2] = vec2(2, -2); offsets[3] = vec2(3, -1);
				offsets[4] = vec2(3, 0); offsets[5] = vec2(3, 1); offsets[6] = vec2(2, 2); offsets[7] = vec2(1, 3);
				offsets[8] = vec2(0, 3); offsets[9] = vec2(-1, 3); offsets[10] = vec2(-2, 2); offsets[11] = vec2(-3, 1);
				offsets[12] = vec2(-3, 0); offsets[13] = vec2(-3, -1); offsets[14] = vec2(-2, -2); offsets[15] = vec2(-1, -3);

				float diffs[16];
				for (int i = 0; i < 16; i++) {
					diffs[i] = texture2D(u_tex, v_uv + offsets[i] * u_texel).r - center;
				}

				int max_run_bright = 0, max_run_dark = 0;
				int run_bright = 0, run_dark = 0;
				for (int i = 0; i < 16 + 9; i++) {
					int idx = i >= 16 ? i - 16 : i;
					float diff = diffs[idx];
					if (diff > u_threshold) {
						run_bright++; run_dark = 0;
					} else if (diff < -u_threshold) {
						run_dark++; run_bright = 0;
					} else {
						run_bright = 0; run_dark = 0;
					}
					if (run_bright > max_run_bright) max_run_bright = run_bright;
					if (run_dark > max_run_dark) max_run_dark = run_dark;
				}

				float score = 0.0;
				if (max_run_bright >= 9 || max_run_dark >= 9) {
					score = max(float(max_run_bright), float(max_run_dark)) / 16.0;
				}
				gl_FragColor = vec4(score, 0.0, 0.0, 1.0);
			}
		)";

		const char* fs_nms = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform vec2 u_texel;
			varying vec2 v_uv;
			void main() {
				float center_score = texture2D(u_tex, v_uv).r;
				if (center_score <= 0.0) {
					gl_FragColor = vec4(0.0);
					return;
				}
				for (int dy = -1; dy <= 1; dy++) {
					for (int dx = -1; dx <= 1; dx++) {
						if (dx == 0 && dy == 0) continue;
						float neighbor_score = texture2D(u_tex, v_uv + vec2(float(dx), float(dy)) * u_texel).r;
						if (neighbor_score > center_score || (neighbor_score == center_score && (dy > 0 || (dy == 0 && dx > 0)))) {
							gl_FragColor = vec4(0.0);
							return;
						}
					}
				}
				gl_FragColor = vec4(center_score, 0.0, 0.0, 0.0);
			}
		)";

		const char* fs_describe = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform sampler2D u_kps;
			uniform sampler2D u_pattern;
			uniform vec2 u_texel;
			uniform float u_kp_v_step;
			varying vec2 v_uv;
			void main() {
				int pix_x = int(gl_FragCoord.x);
				float kp_v = (floor(gl_FragCoord.y) + 0.5) * u_kp_v_step;
				vec4 kp_data = texture2D(u_kps, vec2(0.5, kp_v));
				vec2 kp_pos = kp_data.xy;
				vec4 res = vec4(0.0);
				for (int c = 0; c < 4; c++) {
					int byte_idx = pix_x * 4 + c;
					float val = 0.0;
					float bit_val = 1.0;
					for (int i = 0; i < 8; i++) {
						int bit_idx = byte_idx * 8 + i;
						vec4 p = texture2D(u_pattern, vec2((float(bit_idx) + 0.5) / 256.0, 0.5));
						float t0 = texture2D(u_tex, (kp_pos + p.xy) * u_texel).r;
						float t1 = texture2D(u_tex, (kp_pos + p.zw) * u_texel).r;
						if (t0 < t1) val += bit_val;
						bit_val *= 2.0;
					}
					if (c == 0) res.r = val / 255.0;
					else if (c == 1) res.g = val / 255.0;
					else if (c == 2) res.b = val / 255.0;
					else res.a = val / 255.0;
				}
				gl_FragColor = res;
			}
		)";

		const char* fs_rgba = R"(
			#version 120
			uniform sampler2D u_tex;
			varying vec2 v_uv;
			void main() {
				gl_FragColor = texture2D(u_tex, v_uv);
			}
		)";

		const char* fs_binarize = R"(
			#version 120
			uniform sampler2D u_tex;
			uniform float u_threshold;
			varying vec2 v_uv;
			void main() {
				float gray = texture2D(u_tex, v_uv).r;
				float res = gray > u_threshold ? 1.0 : 0.0;
				gl_FragColor = vec4(res, res, res, 1.0);
			}
		)";

		GLuint vs = CompileShader(GL_VERTEX_SHADER, vs_passthrough, error);
		if (!vs) return false;

		GLuint fs_bin = CompileShader(GL_FRAGMENT_SHADER, fs_binarize, error);
		if (fs_bin) {
			prog_binarize = LinkProgram(vs, fs_bin, error);
			glDeleteShader(fs_bin);
		}

		GLuint fs_r = CompileShader(GL_FRAGMENT_SHADER, fs_rgba, error);
		if (fs_r) {
			prog_rgba = LinkProgram(vs, fs_r, error);
			glDeleteShader(fs_r);
		}

		GLuint fs_gray = CompileShader(GL_FRAGMENT_SHADER, fs_grayscale, error);
		if (!fs_gray) { glDeleteShader(vs); return false; }
		prog_grayscale = LinkProgram(vs, fs_gray, error);
		glDeleteShader(fs_gray);
		if (!prog_grayscale) { glDeleteShader(vs); return false; }

		if (HasExtension("GL_OES_EGL_image_external")) {
			GLuint fs_gray_ext = CompileShader(GL_FRAGMENT_SHADER, fs_grayscale_external, error);
			if (fs_gray_ext) {
				prog_grayscale_external = LinkProgram(vs, fs_gray_ext, error);
				glDeleteShader(fs_gray_ext);
			} else {
				Upp::Cout() << "WARNING: fs_grayscale_external compile failed: " << error << "\n";
				error.Clear(); // Allow continuing without external sampler support
			}
		}

		GLuint fs_yuyv = CompileShader(GL_FRAGMENT_SHADER, fs_yuyv_manual, error);
		if (fs_yuyv) {
			prog_yuyv_manual = LinkProgram(vs, fs_yuyv, error);
			glDeleteShader(fs_yuyv);
		} else {
			Upp::Cout() << "WARNING: fs_yuyv_manual compile failed: " << error << "\n";
			error.Clear();
		}

		GLuint fs_bh = CompileShader(GL_FRAGMENT_SHADER, fs_blur_h, error);
		if (!fs_bh) { glDeleteShader(vs); return false; }
		prog_blur_h = LinkProgram(vs, fs_bh, error);
		glDeleteShader(fs_bh);
		if (!prog_blur_h) { glDeleteShader(vs); return false; }

		GLuint fs_bv = CompileShader(GL_FRAGMENT_SHADER, fs_blur_v, error);
		if (!fs_bv) { glDeleteShader(vs); return false; }
		prog_blur_v = LinkProgram(vs, fs_bv, error);
		glDeleteShader(fs_bv);
		if (!prog_blur_v) { glDeleteShader(vs); return false; }

		GLuint fs_down = CompileShader(GL_FRAGMENT_SHADER, fs_downsample, error);
		if (!fs_down) { glDeleteShader(vs); return false; }
		prog_downsample = LinkProgram(vs, fs_down, error);
		glDeleteShader(fs_down);
		if (!prog_downsample) { glDeleteShader(vs); return false; }

		GLuint fs_f9 = CompileShader(GL_FRAGMENT_SHADER, fs_fast9, error);
		if (!fs_f9) { glDeleteShader(vs); return false; }
		prog_fast9 = LinkProgram(vs, fs_f9, error);
		glDeleteShader(fs_f9);
		if (!prog_fast9) { glDeleteShader(vs); return false; }

		GLuint fs_n = CompileShader(GL_FRAGMENT_SHADER, fs_nms, error);
		if (!fs_n) { glDeleteShader(vs); return false; }
		prog_nms = LinkProgram(vs, fs_n, error);
		glDeleteShader(fs_n);
		if (!prog_nms) { glDeleteShader(vs); return false; }

		GLuint fs_d = CompileShader(GL_FRAGMENT_SHADER, fs_describe, error);
		if (!fs_d) { glDeleteShader(vs); return false; }
		prog_describe = LinkProgram(vs, fs_d, error);
		glDeleteShader(fs_d);
		glDeleteShader(vs);
		if (!prog_describe) return false;

		glGenFramebuffers(1, &fbo);
		return true;
	}

	void CleanupGlTextures() {
		if (tex_rgba) glDeleteTextures(1, &tex_rgba);
		if (tex_gray) glDeleteTextures(1, &tex_gray);
		if (tex_smooth) glDeleteTextures(1, &tex_smooth);
		if (!tex_pyramid_gray.IsEmpty())
			glDeleteTextures(tex_pyramid_gray.GetCount(), tex_pyramid_gray.Begin());
		if (!tex_pyramid_smooth.IsEmpty())
			glDeleteTextures(tex_pyramid_smooth.GetCount(), tex_pyramid_smooth.Begin());
		if (!tex_scoremap.IsEmpty())
			glDeleteTextures(tex_scoremap.GetCount(), tex_scoremap.Begin());
		if (!tex_nms.IsEmpty())
			glDeleteTextures(tex_nms.GetCount(), tex_nms.Begin());
		if (tex_pattern) glDeleteTextures(1, &tex_pattern);
		if (tex_descriptors) glDeleteTextures(1, &tex_descriptors);
		if (tex_kps) glDeleteTextures(1, &tex_kps);
		if (tex_batch) glDeleteTextures(1, &tex_batch);
		// Note: fbo is NOT deleted here as it is tied to the shader lifecycle in GlBackendBase
		tex_rgba = tex_gray = tex_smooth = 0;
		tex_pattern = tex_descriptors = tex_kps = tex_batch = 0;
		tex_width = tex_height = pyramid_levels = 0;
		tex_pyramid_gray.Clear();
		tex_pyramid_smooth.Clear();
		tex_scoremap.Clear();
		tex_nms.Clear();
	}

	void CleanupGlResources() {
		CleanupGlTextures();
		if (prog_grayscale) glDeleteProgram(prog_grayscale);
		if (prog_grayscale_external) glDeleteProgram(prog_grayscale_external);
		if (prog_yuyv_manual) glDeleteProgram(prog_yuyv_manual);
		if (prog_blur_h) glDeleteProgram(prog_blur_h);
		if (prog_blur_v) glDeleteProgram(prog_blur_v);
		if (prog_downsample) glDeleteProgram(prog_downsample);
		if (prog_fast9) glDeleteProgram(prog_fast9);
		if (prog_nms) glDeleteProgram(prog_nms);
		if (prog_describe) glDeleteProgram(prog_describe);
		if (prog_binarize) glDeleteProgram(prog_binarize);
		prog_grayscale = prog_grayscale_external = prog_yuyv_manual = prog_blur_h = prog_blur_v = prog_downsample = prog_fast9 = prog_nms = prog_describe = prog_binarize = 0;
		if (vbo && glDeleteBuffers) glDeleteBuffers(1, &vbo);
		if (vao && glDeleteVertexArrays) glDeleteVertexArrays(1, &vao);
		vbo = vao = 0;
	}

	bool EnsureTextures(int w, int h, int levels, int rgba_w = -1) {
		if (rgba_w < 0) rgba_w = w;
		if (tex_width == w && tex_height == h && tex_rgba_width == rgba_w && pyramid_levels == levels &&
		    tex_rgba && tex_gray && tex_smooth &&
		    tex_pyramid_gray.GetCount() == levels && tex_pyramid_smooth.GetCount() == levels &&
		    tex_scoremap.GetCount() == levels && tex_nms.GetCount() == levels &&
		    tex_pattern && tex_descriptors && tex_kps && tex_batch)
			return true;
		
		CleanupGlTextures();

		glGenTextures(1, &tex_rgba);
		glGenTextures(1, &tex_gray);
		glGenTextures(1, &tex_smooth);
		glGenTextures(1, &tex_pattern);
		glGenTextures(1, &tex_descriptors);
		glGenTextures(1, &tex_kps);
		glGenTextures(1, &tex_batch);

		glBindTexture(GL_TEXTURE_2D, tex_rgba);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgba_w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_gray);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_smooth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_pattern);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 1, 0, GL_RGBA, GL_FLOAT, g_brief_pattern);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_descriptors);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32 / 4, 16384, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_kps);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 16384, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, tex_batch);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		tex_pyramid_gray.SetCount(levels);
		tex_pyramid_smooth.SetCount(levels);
		tex_scoremap.SetCount(levels);
		tex_nms.SetCount(levels);
		glGenTextures(levels, tex_pyramid_gray.Begin());
		glGenTextures(levels, tex_pyramid_smooth.Begin());
		glGenTextures(levels, tex_scoremap.Begin());
		glGenTextures(levels, tex_nms.Begin());

		int pw = w, ph = h;
		for (int i = 0; i < levels; i++) {
			glBindTexture(GL_TEXTURE_2D, tex_pyramid_gray[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_2D, tex_pyramid_smooth[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_2D, tex_scoremap[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glBindTexture(GL_TEXTURE_2D, tex_nms[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pw, ph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		tex_width = w;
		tex_height = h;
		tex_rgba_width = rgba_w;
		pyramid_levels = levels;
		return false;
	}

	void RenderFullscreenQuad(GLuint prog) {
		if (!vao && glGenVertexArrays) {
			glGenVertexArrays(1, &vao);
		}
		if (vao) glBindVertexArray(vao);
		
		if (!vbo && glGenBuffers) {
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			static const float verts[] = {
				-1.0f, -1.0f, 0.0f, 0.0f,
				 1.0f, -1.0f, 1.0f, 0.0f,
				-1.0f,  1.0f, 0.0f, 1.0f,
				 1.0f,  1.0f, 1.0f, 1.0f
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		} else if (vbo) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
		}

		glUseProgram(prog);
		GLint pos_loc = glGetAttribLocation(prog, "pos");
		GLint uv_loc = glGetAttribLocation(prog, "uv");
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		
		// Clear previous errors
		while (glGetError() != GL_NO_ERROR) {}

		if (glCheckFramebufferStatus) {
			GLenum fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
				Cout() << "ERROR: Framebuffer incomplete: " << fb_status << "\n";
			}
		}

		if (pos_loc >= 0) {
			glEnableVertexAttribArray(pos_loc);
			if (vbo) glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			else {
			    static const float verts[] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f };
			    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), verts);
			}
		}
		if (uv_loc >= 0) {
			glEnableVertexAttribArray(uv_loc);
			if (vbo) glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			else {
			    static const float verts[] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f };
			    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), verts + 2);
			}
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			Cout() << "ERROR in RenderFullscreenQuad glDrawArrays: " << err << " (vao=" << vao << " vbo=" << vbo << ")\n";
		}
		if (pos_loc >= 0) glDisableVertexAttribArray(pos_loc);
		if (uv_loc >= 0) glDisableVertexAttribArray(uv_loc);
		if (vbo) glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (vao) glBindVertexArray(0);
		glUseProgram(0);
	}

	void BlurTexture(GLuint src_tex, GLuint dst_tex, int w, int h) {
		GLuint temp_tex;
		glGenTextures(1, &temp_tex);
		glBindTexture(GL_TEXTURE_2D, temp_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, temp_tex, 0);
		glViewport(0, 0, w, h);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, src_tex);
		glUseProgram(prog_blur_h);
		glUniform1i(glGetUniformLocation(prog_blur_h, "u_tex"), 0);
		glUniform2f(glGetUniformLocation(prog_blur_h, "u_texel"), 1.0f / w, 1.0f / h);
		RenderFullscreenQuad(prog_blur_h);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_tex, 0);
		glBindTexture(GL_TEXTURE_2D, temp_tex);
		glUseProgram(prog_blur_v);
		glUniform1i(glGetUniformLocation(prog_blur_v, "u_tex"), 0);
		glUniform2f(glGetUniformLocation(prog_blur_v, "u_texel"), 1.0f / w, 1.0f / h);
		RenderFullscreenQuad(prog_blur_v);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteTextures(1, &temp_tex);
	}

	virtual bool PrepareFrameGpu(const Image& img, int levels, GpuPreprocessStats& stats, String& error) override {
		if (!ready) { error = "Backend not ready"; return false; }
		int w = img.GetWidth();
		int h = img.GetHeight();
		bool reused = EnsureTextures(w, h, levels);
		if (reused) stats.texture_reuse_count++; else stats.texture_realloc_count++;

		TimeStop upload_ts;
		glBindTexture(GL_TEXTURE_2D, tex_rgba);
		upload_format = GL_RGBA;
#ifdef PLATFORM_LINUX
		if (HasExtension("GL_EXT_bgra") || HasExtension("GL_EXT_texture_format_BGRA8888"))
			upload_format = GL_BGRA;
#endif
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, upload_format, GL_UNSIGNED_BYTE, ~img);
		Upp::Cout() << "DEBUG: PrepareFrameGpu upload_format=" << (upload_format == GL_BGRA ? "BGRA" : "RGBA") << "\n";
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			error = "glTexSubImage2D failed: 0x" + FormatIntHex(err);
			glBindTexture(GL_TEXTURE_2D, 0);
			return false;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		
		stats.upload_us = upload_ts.Elapsed();
		stats.upload_copies = 1;
		stats.upload_bytes = (int64)w * (int64)h * 4;
		bool ok = ProcessPreparedRgbaTexture(w, h, levels, stats);
		glFlush();
		return ok;
	}

	// Phase 12 Stage 3: Zero-copy DMA-BUF path (stub for non-EGL backends)
	virtual bool PrepareFrameDmaBuf(const GpuFrame::DmaBufDesc& dmabuf, int pyramid_levels, GpuPreprocessStats& stats, String& error) override {
		error = "DMA-BUF import not implemented for this GL backend";
		return false;
	}

	virtual bool PrepareFrameD3D11(const GpuFrame::D3D11TextureDesc& d3d11, int pyramid_levels, GpuPreprocessStats& stats, String& error) override {
		error = "D3D11 texture import not implemented for this GL backend";
		return false;
	}


	virtual bool ComputeScoreMapsGpu(GpuPreprocessStats& stats, String& error) override {
		if (!ready || tex_pyramid_gray.IsEmpty()) { error = "Backend not ready or pyramid not generated"; return false; }
		TimeStop score_ts;
		int pw = tex_width, ph = tex_height;
		for (int i = 0; i < pyramid_levels; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_scoremap[i], 0);
			glViewport(0, 0, pw, ph);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_pyramid_gray[i]);
			glUseProgram(prog_fast9);
			glUniform1i(glGetUniformLocation(prog_fast9, "u_tex"), 0);
			glUniform2f(glGetUniformLocation(prog_fast9, "u_texel"), 1.0f / pw, 1.0f / ph);
			glUniform1f(glGetUniformLocation(prog_fast9, "u_threshold"), 20.0f / 255.0f);
			
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			
			RenderFullscreenQuad(prog_fast9);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		stats.scoremap_us = score_ts.Elapsed();
		
		TimeStop nms_ts;
		pw = tex_width; ph = tex_height;
		for (int i = 0; i < pyramid_levels; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_nms[i], 0);
			glViewport(0, 0, pw, ph);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_scoremap[i]);
			glUseProgram(prog_nms);
			glUniform1i(glGetUniformLocation(prog_nms, "u_tex"), 0);
			glUniform2f(glGetUniformLocation(prog_nms, "u_texel"), 1.0f / pw, 1.0f / ph);
			RenderFullscreenQuad(prog_nms);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		stats.nms_us = nms_ts.Elapsed();
		return true;
	}

	virtual bool GetKeypointsGpu(Vector<GpuKp>& out, int max_keypoints, String& error) override {
		if (!ready || tex_nms.IsEmpty()) { error = "Backend not ready or NMS not computed"; return false; }
		out.Clear();
		int pw = tex_width, ph = tex_height;
		for (int i = 0; i < pyramid_levels; i++) {
			Vector<byte> nms_data_bytes;
			nms_data_bytes.SetCount(pw * ph);
			glBindTexture(GL_TEXTURE_2D, tex_nms[i]);
			// Read only the RED channel (scores)
			glReadPixels(0, 0, pw, ph, GL_RED, GL_UNSIGNED_BYTE, nms_data_bytes.Begin());
			glBindTexture(GL_TEXTURE_2D, 0);
			
			for (int j = 0; j < pw * ph; j++) {
				float score = nms_data_bytes[j] / 255.0f;
				if (score > 0.0f) {
					int x = j % pw;
					int y = j / pw;
					GpuKp& kp = out.Add();
					kp.score = score;
					kp.x = (float)x + 0.5f;
					kp.y = (float)y + 0.5f;
					kp.level = i;
					if (out.GetCount() >= max_keypoints) break;
				}
			}
			if (out.GetCount() >= max_keypoints) break;
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		return true;
	}

	virtual bool ComputeDescriptorsGpu(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors, String& error) override {
		if (!ready || tex_pyramid_smooth.IsEmpty()) { error = "Backend not ready"; return false; }
		int n = kps.GetCount();
		if (n == 0) { descriptors.Clear(); return true; }
		if (n > 16384) n = 16384;

		// 1. Group keypoints by level
		Vector<int> kps_by_level[16];
		for (int i = 0; i < n; i++) {
			int lev = kps[i].level;
			if (lev >= 0 && lev < 16)
				kps_by_level[lev].Add(i);
		}

		// 2. Upload sorted keypoints
		Vector<float> kp_data;
		kp_data.SetCount(16384 * 4, 0.0f);
		int sorted_idx = 0;
		struct LevelRange { int start, count; };
		LevelRange ranges[16];
		for (int lev = 0; lev < 16; lev++) {
			ranges[lev].start = sorted_idx;
			ranges[lev].count = kps_by_level[lev].GetCount();
			for (int idx : kps_by_level[lev]) {
				if (sorted_idx < 16384) {
					kp_data[sorted_idx * 4 + 0] = kps[idx].x;
					kp_data[sorted_idx * 4 + 1] = kps[idx].y;
					kp_data[sorted_idx * 4 + 2] = (float)lev;
					kp_data[sorted_idx * 4 + 3] = 0.0f;
					sorted_idx++;
				}
			}
		}
		int total_sorted = sorted_idx;
		glBindTexture(GL_TEXTURE_2D, tex_kps);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 16384, GL_RGBA, GL_FLOAT, kp_data.Begin());

		// 3. Render level by level
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_descriptors, 0);
		glViewport(0, 0, 8, 16384);
		
		glDisable(GL_SCISSOR_TEST);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(prog_describe);
		glUniform1i(glGetUniformLocation(prog_describe, "u_kps"), 1);
		glUniform1i(glGetUniformLocation(prog_describe, "u_pattern"), 2);
		glUniform1f(glGetUniformLocation(prog_describe, "u_kp_v_step"), 1.0f / 16384.0f);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex_kps);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex_pattern);
		glActiveTexture(GL_TEXTURE0);

		glEnable(GL_SCISSOR_TEST);
		for (int lev = 0; lev < pyramid_levels; lev++) {
			if (ranges[lev].count == 0) continue;
			
			int pw = tex_width, ph = tex_height;
			for (int i = 0; i < lev; i++) { pw = max(1, pw / 2); ph = max(1, ph / 2); }
			
			glBindTexture(GL_TEXTURE_2D, tex_pyramid_smooth[lev]);
			glUniform2f(glGetUniformLocation(prog_describe, "u_texel"), 1.0f / pw, 1.0f / ph);
			
			glScissor(0, ranges[lev].start, 8, ranges[lev].count);
			RenderFullscreenQuad(prog_describe);
		}
		glDisable(GL_SCISSOR_TEST);

		// 4. Readback
		if (total_sorted > 0) {
			Vector<BinDescriptor> sorted_descriptors;
			sorted_descriptors.SetCount(total_sorted);
			glReadPixels(0, 0, 8, total_sorted, GL_RGBA, GL_UNSIGNED_BYTE, sorted_descriptors[0].u8);
			
			// 5. Re-map sorted descriptors to original order
			descriptors.SetCount(n);
			sorted_idx = 0;
			for (int lev = 0; lev < 16; lev++) {
				for (int orig_idx : kps_by_level[lev]) {
					if (sorted_idx < total_sorted)
						descriptors[orig_idx] = sorted_descriptors[sorted_idx++];
				}
			}
		} else {
			descriptors.Clear();
			descriptors.SetCount(n);
		}

		glUseProgram(0);
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	virtual bool ExtractSparsePatchesGpu(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches, String& error) override {
		if (!ready || tex_pyramid_gray.IsEmpty()) { error = "Backend not ready"; return false; }
		patches.Clear();
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (const GpuKp& kp : kps) {
			if (kp.level < 0 || kp.level >= pyramid_levels) continue;
			
			GpuPatch& patch = patches.Add();
			patch.width = 31;
			patch.height = 31;
			int x0 = (int)floor(kp.x) - 15;
			int y0 = (int)floor(kp.y) - 15;
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_pyramid_gray[kp.level], 0);
			glReadPixels(x0, y0, 31, 31, GL_LUMINANCE, GL_UNSIGNED_BYTE, patch.data);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	virtual bool ReadbackAreasGpu(const Vector<Rect>& rects, Vector<ByteMat>& outcomes, String& error) override {
		if (!ready || !tex_rgba) { error = "Backend not ready"; return false; }
		int n = rects.GetCount();
		outcomes.SetCount(n);
		if (n == 0) return true;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_rgba, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			error = "Framebuffer not complete";
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return false;
		}

		Vector<byte> rgba_buf;
		for (int i = 0; i < n; i++) {
			Rect r = rects[i];
			if (r.left < 0 || r.top < 0 || r.right > tex_width || r.bottom > tex_height) {
				outcomes[i].SetSize(0, 0, 1);
				continue;
			}
			
			int rw = r.GetWidth();
			int rh = r.GetHeight();
			
			outcomes[i].SetSize(rw, rh, 3);
			
			rgba_buf.SetCount(rw * rh * 4);
			// OpenGL uses bottom-left origin, but our texture is uploaded upside down 
			// (top row of Image at bottom row of texture), so r.top maps directly to GL y.
			int gl_y = r.top;
			glReadPixels(r.left, gl_y, rw, rh, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
			
			for (int y = 0; y < rh; y++) {
				for (int x = 0; x < rw; x++) {
					int idx = (y * rw + x) * 4;
					int o_idx = (y * rw + x) * 3;
					bool swap = false;
#ifdef PLATFORM_LINUX
					if (upload_format == GL_RGBA) swap = true;
#endif
					if (swap) {
						outcomes[i].data[o_idx + 0] = rgba_buf[idx + 2]; // R
						outcomes[i].data[o_idx + 1] = rgba_buf[idx + 1]; // G
						outcomes[i].data[o_idx + 2] = rgba_buf[idx + 0]; // B
					} else {
						outcomes[i].data[o_idx + 0] = rgba_buf[idx + 0]; // R
						outcomes[i].data[o_idx + 1] = rgba_buf[idx + 1]; // G
						outcomes[i].data[o_idx + 2] = rgba_buf[idx + 2]; // B
					}
				}
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	virtual bool ReadbackBinarizedAreasGpu(const Vector<Rect>& rects, float threshold, Vector<ByteMat>& outcomes, String& error) override {
		if (!ready || !tex_gray || !prog_binarize) { error = "Backend not ready"; return false; }
		int n = rects.GetCount();
		outcomes.SetCount(n);
		if (n == 0) return true;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_rgba, 0);
		
		glUseProgram(prog_binarize);
		glUniform1i(glGetUniformLocation(prog_binarize, "u_tex"), 0);
		glUniform1f(glGetUniformLocation(prog_binarize, "u_threshold"), threshold);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_gray);

		Vector<byte> rgba_buf;
		for (int i = 0; i < n; i++) {
			Rect r = rects[i];
			if (r.left < 0 || r.top < 0 || r.right > tex_width || r.bottom > tex_height) {
				outcomes[i].SetSize(0, 0, 1);
				continue;
			}
			
			int rw = r.GetWidth();
			int rh = r.GetHeight();
			outcomes[i].SetSize(rw, rh, 1);
			
			glViewport(r.left, r.top, rw, rh);
			glEnable(GL_SCISSOR_TEST);
			glScissor(r.left, r.top, rw, rh);
			RenderFullscreenQuad(prog_binarize);
			glDisable(GL_SCISSOR_TEST);

			rgba_buf.SetCount(rw * rh * 4);
			glReadPixels(r.left, r.top, rw, rh, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
			
			for (int j = 0; j < rw * rh; j++) {
				outcomes[i].data[j] = rgba_buf[j * 4];
			}
		}
		
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}

	virtual bool GetGrayscaleGpu(ByteMat& out, String& error) override {
		if (!ready || !tex_gray) { error = "Backend not ready or no gray texture"; return false; }
		out.SetSize(tex_width, tex_height, 1);
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_gray, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			error = "Framebuffer not complete";
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return false;
		}

		Vector<byte> rgba_buf;
		rgba_buf.SetCount(tex_width * tex_height * 4);
		glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		for (int i = 0; i < tex_width * tex_height; i++)
			out.data[i] = rgba_buf[i * 4];
		return true;
	}

	virtual bool GetSmoothGpu(ByteMat& out, String& error) override {
		if (!ready || !tex_smooth) { error = "Backend not ready or no smooth texture"; return false; }
		out.SetSize(tex_width, tex_height, 1);
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_smooth, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			error = "Framebuffer not complete";
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return false;
		}

		Vector<byte> rgba_buf;
		rgba_buf.SetCount(tex_width * tex_height * 4);
		glReadPixels(0, 0, tex_width, tex_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		for (int i = 0; i < tex_width * tex_height; i++)
			out.data[i] = rgba_buf[i * 4];
		return true;
	}

	virtual bool GetPyramidGrayGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready || tex_pyramid_gray.IsEmpty()) { error = "Backend not ready or no pyramid textures"; return false; }
		out.Clear();
		int pw = tex_width, ph = tex_height;
		for (int i = 0; i < tex_pyramid_gray.GetCount(); i++) {
			ByteMat level;
			level.SetSize(pw, ph, 1);
			Vector<byte> rgba_buf;
			rgba_buf.SetCount(pw * ph * 4);
			glBindTexture(GL_TEXTURE_2D, tex_pyramid_gray[i]);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
			glBindTexture(GL_TEXTURE_2D, 0);
			for (int j = 0; j < pw * ph; j++)
				level.data[j] = rgba_buf[j * 4];
			out.Add(pick(level));
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		return true;
	}

	virtual bool GetPyramidSmoothGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready || tex_pyramid_smooth.IsEmpty()) { error = "Backend not ready or no pyramid textures"; return false; }
		out.Clear();
		int pw = tex_width, ph = tex_height;
		for (int i = 0; i < tex_pyramid_smooth.GetCount(); i++) {
			ByteMat level;
			level.SetSize(pw, ph, 1);
			Vector<byte> rgba_buf;
			rgba_buf.SetCount(pw * ph * 4);
			glBindTexture(GL_TEXTURE_2D, tex_pyramid_smooth[i]);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buf.Begin());
			glBindTexture(GL_TEXTURE_2D, 0);
			for (int j = 0; j < pw * ph; j++)
				level.data[j] = rgba_buf[j * 4];
			out.Add(pick(level));
			pw = max(1, pw / 2);
			ph = max(1, ph / 2);
		}
		return true;
	}
};

class GlxX11Backend : public GlBackendBase {
	::Display* x11_display = nullptr;
	::Window helper_window = 0;
	GLXContext glx_context = nullptr;
	EGLDisplay egl_dmabuf_display = EGL_NO_DISPLAY;
	bool egl_dmabuf_ready = false;
	String egl_dmabuf_error;
	String route = "glx-x11";

public:
	virtual ~GlxX11Backend() {
		Shutdown();
	}

	virtual bool Initialize(const GpuPreprocessConfig& cfg, String& error) override {
		Shutdown();
		x11_display = XOpenDisplay(nullptr);
		if (!x11_display) {
			error = "glx-x11: XOpenDisplay failed (DISPLAY=" + String(GetEnv("DISPLAY")) + ")";
			return false;
		}
		int screen = DefaultScreen(x11_display);
		int visual_attribs[] = {
			GLX_RGBA,
			GLX_DOUBLEBUFFER,
			GLX_RED_SIZE, 8,
			GLX_GREEN_SIZE, 8,
			GLX_BLUE_SIZE, 8,
			GLX_ALPHA_SIZE, 8,
			None
		};
		XVisualInfo* vi = glXChooseVisual(x11_display, screen, visual_attribs);
		if (!vi) {
			error = "glx-x11: glXChooseVisual failed";
			XCloseDisplay(x11_display);
			x11_display = nullptr;
			return false;
		}
		::Window root = RootWindow(x11_display, vi->screen);
		XSetWindowAttributes swa;
		swa.colormap = XCreateColormap(x11_display, root, vi->visual, AllocNone);
		swa.event_mask = 0;
		helper_window = XCreateWindow(x11_display, root, 0, 0, 1, 1, 0,
		                               vi->depth, InputOutput, vi->visual,
		                               CWColormap | CWEventMask, &swa);
		if (!helper_window) {
			error = "glx-x11: XCreateWindow failed";
			XFree(vi);
			XCloseDisplay(x11_display);
			x11_display = nullptr;
			return false;
		}
		glx_context = glXCreateContext(x11_display, vi, nullptr, GL_TRUE);
		XFree(vi);
		if (!glx_context) {
			error = "glx-x11: glXCreateContext failed";
			XDestroyWindow(x11_display, helper_window);
			XCloseDisplay(x11_display);
			x11_display = nullptr;
			helper_window = 0;
			return false;
		}
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) {
			error = "glx-x11: glXMakeCurrent failed";
			glXDestroyContext(x11_display, glx_context);
			XDestroyWindow(x11_display, helper_window);
			XCloseDisplay(x11_display);
			x11_display = nullptr;
			helper_window = 0;
			glx_context = nullptr;
			return false;
		}
		const GLubyte* renderer_str = glGetString(GL_RENDERER);
		if (renderer_str)
			renderer = (const char*)renderer_str;

		LoadGLExtensions();

		// Phase 12: Zero-copy DMA-BUF bridge is disabled by default on this system 
		// because EGL is not practically supported and eglQueryString crashes.
		egl_dmabuf_ready = false;
		egl_dmabuf_display = EGL_NO_DISPLAY;
		egl_dmabuf_error = "EGL bridge disabled (system does not support EGL in practice)";

		if (!InitShaders(error)) {
			glXMakeCurrent(x11_display, None, nullptr);
			if (egl_dmabuf_display != EGL_NO_DISPLAY) {
				eglTerminate(egl_dmabuf_display);
				egl_dmabuf_display = EGL_NO_DISPLAY;
				egl_dmabuf_ready = false;
			}
			glXDestroyContext(x11_display, glx_context);
			XDestroyWindow(x11_display, helper_window);
			XCloseDisplay(x11_display);
			x11_display = nullptr;
			helper_window = 0;
			glx_context = nullptr;
			return false;
		}

		glXMakeCurrent(x11_display, None, nullptr);
		ready = true;
		LOG("gpu_backend_init route=glx-x11 renderer=\"" << renderer << "\" helper_window=" << (int)helper_window);
		return true;
	}

	virtual void Shutdown() override {
		if (ready) {
			LOG("gpu_backend_shutdown route=glx-x11");
		}
		if (glx_context) {
			if (glXMakeCurrent(x11_display, helper_window, glx_context)) {
				CleanupGlResources();
				glXMakeCurrent(x11_display, None, nullptr);
			}
			glXDestroyContext(x11_display, glx_context);
			glx_context = nullptr;
		}
		if (egl_dmabuf_display != EGL_NO_DISPLAY) {
			eglTerminate(egl_dmabuf_display);
			egl_dmabuf_display = EGL_NO_DISPLAY;
		}
		egl_dmabuf_ready = false;
		egl_dmabuf_error.Clear();
		if (helper_window) {
			XDestroyWindow(x11_display, helper_window);
			helper_window = 0;
		}
		if (x11_display) {
			XCloseDisplay(x11_display);
			x11_display = nullptr;
		}
		ready = false;
		renderer.Clear();
	}

	virtual String GetRoute() const override { return route; }
	virtual void MakeCurrent() override { if (ready) glXMakeCurrent(x11_display, helper_window, glx_context); }

	virtual bool PrepareFrameGpu(const Image& img, int levels, GpuPreprocessStats& stats, String& error) override {
		if (!ready) return false;
		glXMakeCurrent(x11_display, helper_window, glx_context);
		return GlBackendBase::PrepareFrameGpu(img, levels, stats, error);
	}

	virtual bool PrepareFrameDmaBuf(const GpuFrame::DmaBufDesc& dmabuf, int levels, GpuPreprocessStats& stats, String& error) override {
		if (!ready) { error = "Backend not ready"; return false; }
		if (dmabuf.fd < 0) { error = "Invalid DMA-BUF fd"; return false; }
		
		GpuFrame::ExternalFormat external_format = dmabuf.external_format != GpuFrame::EXTERNAL_UNKNOWN
			? dmabuf.external_format
			: InferExternalFormatFromDmaBuf(dmabuf.fourcc);

		int w = dmabuf.width;
		int h = dmabuf.height;

		if (external_format == GpuFrame::EXTERNAL_YUYV422) {
			if (egl_dmabuf_ready && egl_dmabuf_display != EGL_NO_DISPLAY && prog_yuyv_manual) {
				if (!glXMakeCurrent(x11_display, helper_window, glx_context)) {
					error = "glXMakeCurrent failed";
					return false;
				}
				bool reused = EnsureTextures(w, h, levels, w / 2);
				if (reused) stats.texture_reuse_count++; else stats.texture_realloc_count++;

				TimeStop upload_ts;
				EGLImageKHR egl_image = ImportDmaBufAsEGLImage(egl_dmabuf_display, dmabuf.fd, w / 2, h, DRM_FORMAT_ABGR8888,
				                                               dmabuf.stride, dmabuf.modifier);
				if (egl_image != EGL_NO_IMAGE_KHR) {
					if (BindEGLImageToTexture(tex_rgba, egl_image, GL_TEXTURE_2D)) {
						DestroyEglImage(egl_dmabuf_display, egl_image);
						stats.upload_us = upload_ts.Elapsed();
						stats.upload_copies = 0;
						stats.upload_bytes = 0;
						bool ok = ProcessPreparedYuyvManualTexture(w, h, levels, stats);
						glXMakeCurrent(x11_display, None, nullptr);
						return ok;
					}
					DestroyEglImage(egl_dmabuf_display, egl_image);
				}
			}
			// Low-copy mmap fallback
			if (!glXMakeCurrent(x11_display, helper_window, glx_context)) {
				error = "glXMakeCurrent failed";
				return false;
			}
			bool reused = EnsureTextures(w, h, levels, w / 2);
			if (reused) stats.texture_reuse_count++; else stats.texture_realloc_count++;

			int map_stride = dmabuf.stride > 0 ? dmabuf.stride : w * 2;
			size_t map_size = (size_t)map_stride * (size_t)h;
			void* mapped = mmap(nullptr, map_size, PROT_READ, MAP_SHARED, dmabuf.fd, 0);
			if (mapped == MAP_FAILED) {
				error = String("glx-x11 YUYV low-copy mmap failed: ") + strerror(errno);
				glXMakeCurrent(x11_display, None, nullptr);
				return false;
			}
			
			TimeStop upload_ts;
			glBindTexture(GL_TEXTURE_2D, tex_rgba);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, map_stride / 4);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w / 2, h, GL_RGBA, GL_UNSIGNED_BYTE, mapped);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			munmap(mapped, map_size);
			
			stats.upload_us = upload_ts.Elapsed();
			stats.upload_copies = 1;
			stats.upload_bytes = (int64)map_stride * (int64)h;
			
			bool ok = ProcessPreparedYuyvManualTexture(w, h, levels, stats);
			glXMakeCurrent(x11_display, None, nullptr);
			return ok;
		}

		if (!egl_dmabuf_ready || egl_dmabuf_display == EGL_NO_DISPLAY) {
			error = egl_dmabuf_error.IsEmpty() ? String("glx-x11 DMA-BUF bridge unavailable") : egl_dmabuf_error;
			return false;
		}

		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) {
			error = "glXMakeCurrent failed";
			return false;
		}

		GLenum target = (external_format == GpuFrame::EXTERNAL_YUYV422) ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;

		if (!IsDirectRgbaLikeExternalFormat(external_format) && external_format != GpuFrame::EXTERNAL_YUYV422) {
			error = String("glx-x11 DMA-BUF import has no safe handler for external format ") +
			        GetExternalFormatName(external_format) + " (fourcc=0x" + FormatIntHex((int)dmabuf.fourcc) + ")";
			glXMakeCurrent(x11_display, None, nullptr);
			return false;
		}

		TimeStop upload_ts;
		EGLImageKHR egl_image = ImportDmaBufAsEGLImage(egl_dmabuf_display, dmabuf.fd, w, h, dmabuf.fourcc,
		                                               dmabuf.stride, dmabuf.modifier);
		if (egl_image == EGL_NO_IMAGE_KHR) {
			error = "ImportDmaBufAsEGLImage failed (error=0x" + FormatIntHex(eglGetError()) + ")";
			glXMakeCurrent(x11_display, None, nullptr);
			return false;
		}
		if (!BindEGLImageToTexture(tex_rgba, egl_image, target)) {
			DestroyEglImage(egl_dmabuf_display, egl_image);
			error = "BindEGLImageToTexture failed (error=0x" + FormatIntHex(glGetError()) + ")";
			glXMakeCurrent(x11_display, None, nullptr);
			return false;
		}
		DestroyEglImage(egl_dmabuf_display, egl_image);
		stats.upload_us = upload_ts.Elapsed();
		stats.upload_copies = 0;
		stats.upload_bytes = 0;
		bool ok = (target == GL_TEXTURE_EXTERNAL_OES) ?
		            ProcessPreparedExternalTexture(w, h, levels, stats) :
		            ProcessPreparedRgbaTexture(w, h, levels, stats);
	}

	virtual bool PrepareFrameD3D11(const GpuFrame::D3D11TextureDesc& d3d11, int levels, GpuPreprocessStats& stats, String& error) override {
		error = "D3D11 texture import not supported on glx-x11 backend";
		return false;
	}

	virtual bool ComputeScoreMapsGpu(GpuPreprocessStats& stats, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::ComputeScoreMapsGpu(stats, error);
	}

	virtual bool GetKeypointsGpu(Vector<GpuKp>& out, int max_keypoints, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::GetKeypointsGpu(out, max_keypoints, error);
	}

	virtual bool ComputeDescriptorsGpu(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::ComputeDescriptorsGpu(kps, descriptors, error);
	}

	virtual bool ExtractSparsePatchesGpu(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::ExtractSparsePatchesGpu(kps, patches, error);
	}

	virtual bool ReadbackAreasGpu(const Vector<Rect>& rects, Vector<ByteMat>& outcomes, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::ReadbackAreasGpu(rects, outcomes, error);
	}

	virtual bool GetGrayscaleGpu(ByteMat& out, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::GetGrayscaleGpu(out, error);
	}

	virtual bool GetSmoothGpu(ByteMat& out, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::GetSmoothGpu(out, error);
	}

	virtual bool GetPyramidGrayGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::GetPyramidGrayGpu(out, error);
	}

	virtual bool GetPyramidSmoothGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready) return false;
		if (!glXMakeCurrent(x11_display, helper_window, glx_context)) return false;
		return GlBackendBase::GetPyramidSmoothGpu(out, error);
	}
};

class EglBackendBase : public GlBackendBase {
protected:
	EGLDisplay egl_display = EGL_NO_DISPLAY;
	EGLSurface egl_surface = EGL_NO_SURFACE;
	EGLContext egl_context = EGL_NO_CONTEXT;

public:
	virtual void Shutdown() override {
		if (egl_display != EGL_NO_DISPLAY) {
			if (egl_context != EGL_NO_CONTEXT) {
				if (eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
					CleanupGlResources();
					eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
				}
				eglDestroyContext(egl_display, egl_context);
			}
			if (egl_surface != EGL_NO_SURFACE)
				eglDestroySurface(egl_display, egl_surface);
			eglTerminate(egl_display);
		}
		egl_display = EGL_NO_DISPLAY;
		egl_surface = EGL_NO_SURFACE;
		egl_context = EGL_NO_CONTEXT;
		ready = false;
		renderer.Clear();
	}

	virtual void MakeCurrent() override { if (ready) eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context); }

	virtual bool PrepareFrameGpu(const Image& img, int levels, GpuPreprocessStats& stats, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::PrepareFrameGpu(img, levels, stats, error);
	}

	// Phase 12 Stage 3: Zero-copy DMA-BUF import via EGLImage
	virtual bool PrepareFrameDmaBuf(const GpuFrame::DmaBufDesc& dmabuf, int levels, GpuPreprocessStats& stats, String& error) override {
		if (!ready) { error = "Backend not ready"; return false; }
		if (dmabuf.fd < 0) { error = "Invalid DMA-BUF fd"; return false; }
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
			error = "eglMakeCurrent failed";
			return false;
		}

		GpuFrame::ExternalFormat external_format = dmabuf.external_format != GpuFrame::EXTERNAL_UNKNOWN
			? dmabuf.external_format
			: InferExternalFormatFromDmaBuf(dmabuf.fourcc);

		int w = dmabuf.width;
		int h = dmabuf.height;
		bool reused = EnsureTextures(w, h, levels);
		if (reused) stats.texture_reuse_count++; else stats.texture_realloc_count++;

		TimeStop upload_ts;

		// Import DMA-BUF as EGLImage
		EGLImageKHR egl_image = ImportDmaBufAsEGLImage(egl_display, dmabuf.fd, w, h, dmabuf.fourcc,
		                                               dmabuf.stride, dmabuf.modifier);
		if (egl_image == EGL_NO_IMAGE_KHR) {
			eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			error = "ImportDmaBufAsEGLImage failed (error=0x" + FormatIntHex(eglGetError()) + ")";
			return false;
		}

		GLenum target = (external_format == GpuFrame::EXTERNAL_YUYV422) ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D;

		// Bind EGLImage to tex_rgba (zero-copy!)
		if (!BindEGLImageToTexture(tex_rgba, egl_image, target)) {
			DestroyEglImage(egl_display, egl_image);
			eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			error = "BindEGLImageToTexture failed (error=0x" + FormatIntHex(glGetError()) + ")";
			return false;
		}

		DestroyEglImage(egl_display, egl_image);  // Can destroy after binding

		stats.upload_us = upload_ts.Elapsed();
		stats.upload_copies = 0;  // Zero-copy!
		stats.upload_bytes = 0;   // No data uploaded!

		bool ok = (target == GL_TEXTURE_EXTERNAL_OES) ?
		            ProcessPreparedExternalTexture(w, h, levels, stats) :
		            ProcessPreparedRgbaTexture(w, h, levels, stats);

		return ok;
	}

	virtual bool PrepareFrameD3D11(const GpuFrame::D3D11TextureDesc& d3d11, int levels, GpuPreprocessStats& stats, String& error) override {
		error = "D3D11 texture import not supported on EGL backend";
		return false;
	}

	virtual bool ComputeScoreMapsGpu(GpuPreprocessStats& stats, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::ComputeScoreMapsGpu(stats, error);
	}

	virtual bool GetKeypointsGpu(Vector<GpuKp>& out, int max_keypoints, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::GetKeypointsGpu(out, max_keypoints, error);
	}

	virtual bool ComputeDescriptorsGpu(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::ComputeDescriptorsGpu(kps, descriptors, error);
	}

	virtual bool ExtractSparsePatchesGpu(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::ExtractSparsePatchesGpu(kps, patches, error);
	}

	virtual bool ReadbackAreasGpu(const Vector<Rect>& rects, Vector<ByteMat>& outcomes, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::ReadbackAreasGpu(rects, outcomes, error);
	}

	virtual bool GetGrayscaleGpu(ByteMat& out, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::GetGrayscaleGpu(out, error);
	}

	virtual bool GetSmoothGpu(ByteMat& out, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::GetSmoothGpu(out, error);
	}

	virtual bool GetPyramidGrayGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::GetPyramidGrayGpu(out, error);
	}

	virtual bool GetPyramidSmoothGpu(Vector<ByteMat>& out, String& error) override {
		if (!ready) return false;
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) return false;
		return GlBackendBase::GetPyramidSmoothGpu(out, error);
	}
};

class EglSurfacelessBackend : public EglBackendBase {
	String route = "egl-surfaceless";

public:
	virtual bool Initialize(const GpuPreprocessConfig& cfg, String& error) override {
		Shutdown();
		PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display =
			(PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
		if (!get_platform_display) {
			error = "egl-surfaceless: eglGetPlatformDisplayEXT unavailable";
			return false;
		}
		egl_display = get_platform_display(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
		if (egl_display == EGL_NO_DISPLAY) {
			error = "egl-surfaceless: eglGetPlatformDisplayEXT failed";
			return false;
		}
		EGLint major, minor;
		if (!eglInitialize(egl_display, &major, &minor)) {
			error = "egl-surfaceless: eglInitialize failed";
			return false;
		}
		EGLConfig egl_cfg;
		if (!ChooseProbeConfig(egl_display, egl_cfg)) {
			error = "egl-surfaceless: eglChooseConfig failed";
			return false;
		}
		if (!eglBindAPI(EGL_OPENGL_API)) {
			error = "egl-surfaceless: eglBindAPI failed";
			return false;
		}
		const EGLint ctx_attrs[] = {
			0x3098, 3, // EGL_CONTEXT_MAJOR_VERSION
			0x30FB, 3, // EGL_CONTEXT_MINOR_VERSION
			0x30FD, 0x00000001, // EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
			EGL_NONE
		};
		egl_context = eglCreateContext(egl_display, egl_cfg, EGL_NO_CONTEXT, ctx_attrs);
		if (egl_context == EGL_NO_CONTEXT) {
			egl_context = eglCreateContext(egl_display, egl_cfg, EGL_NO_CONTEXT, nullptr); // Fallback
		}
		if (egl_context == EGL_NO_CONTEXT) {
			error = "egl-surfaceless: eglCreateContext failed";
			return false;
		}
		const EGLint pbuf_attrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
		egl_surface = eglCreatePbufferSurface(egl_display, egl_cfg, pbuf_attrs);
		if (egl_surface == EGL_NO_SURFACE) {
			error = "egl-surfaceless: eglCreatePbufferSurface failed";
			return false;
		}
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
			error = "egl-surfaceless: eglMakeCurrent failed";
			return false;
		}
		const GLubyte* s = glGetString(GL_RENDERER);
		if (s) renderer = (const char*)s;
		
		LoadGLExtensions();
		if (!InitShaders(error)) return false;

		ready = true;
		LOG("gpu_backend_init route=egl-surfaceless renderer=\"" << renderer << "\"");
		return true;
	}

	virtual String GetRoute() const override { return route; }
};

class EglX11Backend : public EglBackendBase {
	::Display* x11_display = nullptr;
	String route = "egl-x11";

public:
	virtual bool Initialize(const GpuPreprocessConfig& cfg, String& error) override {
		Shutdown();
		x11_display = XOpenDisplay(nullptr);
		if (!x11_display) {
			error = "egl-x11: XOpenDisplay failed";
			return false;
		}
		egl_display = eglGetDisplay((EGLNativeDisplayType)x11_display);
		if (egl_display == EGL_NO_DISPLAY) {
			error = "egl-x11: eglGetDisplay failed";
			return false;
		}
		EGLint major, minor;
		if (!eglInitialize(egl_display, &major, &minor)) {
			error = "egl-x11: eglInitialize failed";
			return false;
		}
		EGLConfig egl_cfg;
		if (!ChooseProbeConfig(egl_display, egl_cfg)) {
			error = "egl-x11: eglChooseConfig failed";
			return false;
		}
		if (!eglBindAPI(EGL_OPENGL_API)) {
			error = "egl-x11: eglBindAPI failed";
			return false;
		}
		const EGLint ctx_attrs[] = {
			0x3098, 3, // EGL_CONTEXT_MAJOR_VERSION
			0x30FB, 3, // EGL_CONTEXT_MINOR_VERSION
			0x30FD, 0x00000001, // EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
			EGL_NONE
		};
		egl_context = eglCreateContext(egl_display, egl_cfg, EGL_NO_CONTEXT, ctx_attrs);
		if (egl_context == EGL_NO_CONTEXT) {
			egl_context = eglCreateContext(egl_display, egl_cfg, EGL_NO_CONTEXT, nullptr); // Fallback
		}
		if (egl_context == EGL_NO_CONTEXT) {
			error = "egl-x11: eglCreateContext failed";
			return false;
		}
		const EGLint pbuf_attrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
		egl_surface = eglCreatePbufferSurface(egl_display, egl_cfg, pbuf_attrs);
		if (egl_surface == EGL_NO_SURFACE) {
			error = "egl-x11: eglCreatePbufferSurface failed";
			return false;
		}
		if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
			error = "egl-x11: eglMakeCurrent failed";
			return false;
		}
		const GLubyte* s = glGetString(GL_RENDERER);
		if (s) renderer = (const char*)s;
		
		LoadGLExtensions();
		if (!InitShaders(error)) return false;

		ready = true;
		LOG("gpu_backend_init route=egl-x11 renderer=\"" << renderer << "\"");
		return true;
	}

	virtual void Shutdown() override {
		EglBackendBase::Shutdown();
		if (x11_display) {
			XCloseDisplay(x11_display);
			x11_display = nullptr;
		}
	}

	virtual String GetRoute() const override { return route; }
};

static EglProbeResult ProbeEglSurfaceless() {
	EglProbeResult out;
	PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display =
		(PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
	if (!get_platform_display) {
		out.error = "eglGetPlatformDisplayEXT unavailable";
		return out;
	}
	EGLDisplay dpy = get_platform_display(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
	if (dpy == EGL_NO_DISPLAY) {
		out.error = "surfaceless display unavailable";
		return out;
	}
	EGLint major = 0, minor = 0;
	if (eglInitialize(dpy, &major, &minor) != EGL_TRUE) {
		out.error = "eglInitialize surfaceless failed";
		return out;
	}
	EGLConfig cfg = nullptr;
	if (!ChooseProbeConfig(dpy, cfg)) {
		out.error = "eglChooseConfig surfaceless failed";
		eglTerminate(dpy);
		return out;
	}
	String renderer, error;
	out.ok = MakeProbeContext(dpy, cfg, renderer, error);
	out.route = "egl-surfaceless";
	out.renderer = renderer;
	out.error = error;
	eglTerminate(dpy);
	return out;
}

static EglProbeResult ProbeEglX11() {
	EglProbeResult out;
	if (IsNull(GetEnv("DISPLAY")) || TrimBoth(GetEnv("DISPLAY")).IsEmpty()) {
		out.error = "DISPLAY unavailable";
		return out;
	}
	::Display* x11 = XOpenDisplay(nullptr);
	if (!x11) {
		out.error = "XOpenDisplay failed";
		return out;
	}
	EGLDisplay dpy = eglGetDisplay((EGLNativeDisplayType)x11);
	if (dpy == EGL_NO_DISPLAY) {
		out.error = "eglGetDisplay x11 failed";
		XCloseDisplay(x11);
		return out;
	}
	EGLint major = 0, minor = 0;
	if (eglInitialize(dpy, &major, &minor) != EGL_TRUE) {
		out.error = "eglInitialize x11 failed";
		eglTerminate(dpy);
		XCloseDisplay(x11);
		return out;
	}
	EGLConfig cfg = nullptr;
	if (!ChooseProbeConfig(dpy, cfg)) {
		out.error = "eglChooseConfig x11 failed";
		eglTerminate(dpy);
		XCloseDisplay(x11);
		return out;
	}
	String renderer, error;
	out.ok = MakeProbeContext(dpy, cfg, renderer, error);
	out.route = "egl-x11";
	out.renderer = renderer;
	out.error = error;
	eglTerminate(dpy);
	XCloseDisplay(x11);
	return out;
}

static EglProbeResult ProbeEglRuntime() {
	GpuBackendProbe cap = GpuPreprocessEngine::ProbeCapabilities();
	if (cap.preferred_backend == "egl-gl") {
		EglProbeResult ok;
		ok.ok = true;
		ok.route = cap.preferred_route;
		ok.renderer = cap.preferred_renderer;
		return ok;
	}
	EglProbeResult surf = ProbeEglSurfaceless();
	if (surf.ok)
		return surf;
	EglProbeResult x11 = ProbeEglX11();
	if (x11.ok)
		return x11;
	EglProbeResult fail;
	fail.error = "surfaceless: " + surf.error + "; x11: " + x11.error;
	return fail;
}

static String RunFirstAvailableCommand(const Vector<String>& cmds) {
	for (const String& cmdline : cmds) {
		Vector<String> parts = Split(cmdline, '\t');
		if (parts.IsEmpty())
			continue;
		String exe = parts[0];
		Vector<String> args;
		for (int i = 1; i < parts.GetCount(); i++)
			args.Add(parts[i]);
		String out = Sys(exe, args, false);
		if (!IsNull(out) && !out.IsEmpty())
			return out;
	}
	return String();
}

static GpuBackendProbe BuildCapabilityProbe() {
	GpuBackendProbe probe;
	String probe_out = RunFirstAvailableCommand({
		String("eglinfo"),
		String("/usr/bin/eglinfo"),
		String("/bin/eglinfo")
	});
	probe.has_eglinfo = !IsNull(probe_out) && !probe_out.IsEmpty();
	if (probe.has_eglinfo) {
		Vector<String> lines = Split(probe_out, '\n');
		bool surf = false;
		bool x11 = false;
		String surf_renderer, surf_driver, x11_renderer, x11_driver;
		for (String line : lines) {
			line = TrimBoth(line);
			if (line == "Surfaceless platform:") {
				surf = true;
				x11 = false;
				Cout() << "DEBUG: BuildCapabilityProbe: found Surfaceless platform\n";
				continue;
			}
			else if (line == "X11 platform:") {
				x11 = true;
				surf = false;
				Cout() << "DEBUG: BuildCapabilityProbe: found X11 platform\n";
				continue;
			}
			else if (line.EndsWith("platform:")) {
				surf = false;
				x11 = false;
				continue;
			}
			if (surf) {
				if (line.StartsWith("EGL driver name:")) {
					surf_driver = TrimBoth(line.Mid((int)strlen("EGL driver name:")));
					Cout() << "DEBUG: BuildCapabilityProbe: surf_driver=" << surf_driver << "\n";
				}
				if (line.StartsWith("OpenGL core profile renderer:")) {
					surf_renderer = TrimBoth(line.Mid((int)strlen("OpenGL core profile renderer:")));
					Cout() << "DEBUG: BuildCapabilityProbe: surf_renderer=" << surf_renderer << "\n";
				}
			}
			if (x11) {
				if (line.StartsWith("EGL driver name:")) {
					x11_driver = TrimBoth(line.Mid((int)strlen("EGL driver name:")));
					Cout() << "DEBUG: BuildCapabilityProbe: x11_driver=" << x11_driver << "\n";
				}
				if (line.StartsWith("OpenGL core profile renderer:")) {
					x11_renderer = TrimBoth(line.Mid((int)strlen("OpenGL core profile renderer:")));
					Cout() << "DEBUG: BuildCapabilityProbe: x11_renderer=" << x11_renderer << "\n";
				}
			}
		}
		probe.surfaceless_renderer = surf_renderer;
		probe.x11_renderer = x11_renderer;
		probe.egl_surfaceless_ok = !surf_renderer.IsEmpty();
		probe.egl_x11_ok = !x11_renderer.IsEmpty() && x11_driver != "swrast";
	}
	
	if (!probe.egl_surfaceless_ok) {
		EglProbeResult res = ProbeEglSurfaceless();
		if (res.ok) {
			probe.egl_surfaceless_ok = true;
			probe.surfaceless_renderer = res.renderer;
		}
	}
	if (!probe.egl_x11_ok) {
		EglProbeResult res = ProbeEglX11();
		if (res.ok) {
			probe.egl_x11_ok = true;
			probe.x11_renderer = res.renderer;
		}
	}

	String glx = RunFirstAvailableCommand({
		String("glxinfo\t-B"),
		String("/usr/bin/glxinfo\t-B"),
		String("/bin/glxinfo\t-B")
	});
	probe.has_glxinfo = !IsNull(glx) && !glx.IsEmpty();
	if (probe.has_glxinfo && glx.Find("direct rendering: Yes") >= 0) {
		int p = glx.Find("OpenGL renderer string:");
		if (p >= 0) {
			int e = glx.Find('\n', p);
			probe.glx_renderer = TrimBoth(glx.Mid(p + (int)strlen("OpenGL renderer string:"), e < 0 ? glx.GetCount() : e - p - (int)strlen("OpenGL renderer string:")));
		}
		probe.glx_x11_ok = !probe.glx_renderer.IsEmpty();
	}
	if (probe.glx_x11_ok) {
		probe.preferred_backend = "egl-gl";
		probe.preferred_route = "glx-x11";
		probe.preferred_renderer = probe.glx_renderer;
	}
	else if (probe.egl_surfaceless_ok) {
		probe.preferred_backend = "egl-gl";
		probe.preferred_route = "egl-surfaceless";
		probe.preferred_renderer = probe.surfaceless_renderer;
	}
	else if (probe.egl_x11_ok) {
		probe.preferred_backend = "egl-gl";
		probe.preferred_route = "egl-x11";
		probe.preferred_renderer = probe.x11_renderer;
	}
	else {
		probe.preferred_backend = "stub";
		probe.preferred_route = "stub-cpu";
		probe.probe_error = "No accelerated EGL/GLX route found";
	}
	return probe;
}

#endif

GpuBackendProbe GpuPreprocessEngine::ProbeCapabilities(bool refresh) {
#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)
	static GpuBackendProbe cache;
	static bool cache_valid = false;
	static Mutex lock;
	Mutex::Lock __(lock);
	if (!cache_valid || refresh) {
		cache = BuildCapabilityProbe();
		cache_valid = true;
	}
	return cache;
#else
	GpuBackendProbe probe;
	probe.preferred_backend = "stub";
	probe.preferred_route = "stub-cpu";
	probe.probe_error = "GPU probe not implemented on this platform";
	return probe;
#endif
}

String GpuPreprocessEngine::FormatProbeReport(const GpuBackendProbe& p) {
	String out;
	out << "gpu_probe preferred_backend=" << p.preferred_backend
	    << " preferred_route=" << p.preferred_route
	    << " has_eglinfo=" << (p.has_eglinfo ? "1" : "0")
	    << " has_glxinfo=" << (p.has_glxinfo ? "1" : "0")
	    << " egl_surfaceless_ok=" << (p.egl_surfaceless_ok ? "1" : "0")
	    << " egl_x11_ok=" << (p.egl_x11_ok ? "1" : "0")
	    << " glx_x11_ok=" << (p.glx_x11_ok ? "1" : "0") << "\n";
	if (!p.preferred_renderer.IsEmpty())
		out << "gpu_probe renderer=\"" << p.preferred_renderer << "\"\n";
	if (!p.surfaceless_renderer.IsEmpty())
		out << "gpu_probe surfaceless_renderer=\"" << p.surfaceless_renderer << "\"\n";
	if (!p.x11_renderer.IsEmpty())
		out << "gpu_probe x11_renderer=\"" << p.x11_renderer << "\"\n";
	if (!p.glx_renderer.IsEmpty())
		out << "gpu_probe glx_renderer=\"" << p.glx_renderer << "\"\n";
	if (!p.probe_error.IsEmpty())
		out << "gpu_probe_error=" << p.probe_error << "\n";
	return out;
}

String GpuPreprocessEngine::NormalizeBackend(String s) {
	s = ToLower(TrimBoth(s));
	if (s.IsEmpty())
		return "auto";
	if (s == "cpu" || s == "cpu-stub")
		return "stub";
	if (s == "egl" || s == "egl-gl" || s == "opengl" || s == "gl")
		return "egl-gl";
	if (s == "automatic")
		return "auto";
	return s;
}

String GpuPreprocessEngine::ResolveBackend(String s) {
	s = NormalizeBackend(s);
	if (s == "auto")
		return "stub";
	return s;
}

String GpuPreprocessEngine::NormalizeRoute(String s) {
	s = ToLower(TrimBoth(s));
	if (s.IsEmpty() || s == "default")
		return "auto";
	if (s == "surfaceless")
		return "egl-surfaceless";
	if (s == "x11")
		return "egl-x11";
	if (s == "glx" || s == "helper-window")
		return "glx-x11";
	if (s == "cpu" || s == "stub")
		return "stub-cpu";
	return s;
}

Vector<String> GpuPreprocessEngine::GetRouteNames() {
	Vector<String> out;
	out.Add("auto");
	out.Add("egl-surfaceless");
	out.Add("egl-x11");
	out.Add("glx-x11");
	out.Add("stub-cpu");
	return out;
}

Vector<String> GpuPreprocessEngine::GetBackendNames() {
	Vector<String> out;
	out.Add("auto");
	out.Add("stub");
	out.Add("egl-gl");
	return out;
}

static void BuildCpuPyramid(const ByteMat& src_gray, const ByteMat& src_smooth,
                            int levels, Vector<ByteMat>& out_gray, Vector<ByteMat>& out_smooth) {
	out_gray.Clear();
	out_smooth.Clear();
	if (src_gray.IsEmpty() || src_smooth.IsEmpty() || levels <= 0)
		return;
	out_gray.Add(clone(src_gray));
	out_smooth.Add(clone(src_smooth));
	for (int i = 1; i < levels; i++) {
		const ByteMat& prev = out_gray.Top();
		if (prev.cols <= 1 || prev.rows <= 1)
			break;
		ByteMat next_gray(max(1, prev.cols / 2), max(1, prev.rows / 2), 1);
		DownsamplePyramid(prev, next_gray);
		ByteMat next_smooth(next_gray.cols, next_gray.rows, 1);
		GaussianBlur(next_gray, next_smooth, 9);
		out_gray.Add(pick(next_gray));
		out_smooth.Add(pick(next_smooth));
	}
}

GpuPreprocessEngine::GpuPreprocessEngine() {
}

GpuPreprocessEngine::~GpuPreprocessEngine() {
	Shutdown();
}

bool GpuPreprocessEngine::Initialize(const GpuPreprocessConfig& c) {
	cfg = c;
	stats = GpuPreprocessStats();
	backend.Clear();
	cfg.backend = NormalizeBackend(cfg.backend);
	cfg.route = NormalizeRoute(cfg.route);
	stats.requested_backend = cfg.backend;
	stats.requested_route = cfg.route;
	if (cfg.backend == "auto") {
#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)
		stats.backend = "egl-gl";
		stats.backend_route = "auto-select-egl";
#else
		stats.backend = "stub";
		stats.backend_route = "auto-select-stub";
#endif
	}
	else {
		stats.backend = ResolveBackend(cfg.backend);
	}
	if (stats.backend == "stub" && stats.backend_route.IsEmpty())
		stats.backend_route = "stub-cpu";
	stats.renderer.Clear();
	if (stats.backend != "stub" && stats.backend != "egl-gl") {
		stats.last_error = "Unsupported backend: " + stats.backend;
		initialized = false;
		return false;
	}
	if (stats.backend == "egl-gl") {
#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)
		GpuBackendProbe cap = ProbeCapabilities();
		String route = cfg.route;
		String resolved_route;
		if (route == "auto") {
			if (cap.glx_x11_ok)
				resolved_route = "glx-x11";
			else if (cap.egl_surfaceless_ok)
				resolved_route = "egl-surfaceless";
			else if (cap.egl_x11_ok)
				resolved_route = "egl-x11";
			else {
				stats.backend = "stub";
				stats.backend_route = "auto-fallback-stub";
				stats.last_error = "No GPU route available";
			}
		}
		else if (route == "egl-surfaceless") {
			if (cap.egl_surfaceless_ok)
				resolved_route = "egl-surfaceless";
			else
				stats.last_error = "requested route unavailable: egl-surfaceless";
		}
		else if (route == "egl-x11") {
			if (cap.egl_x11_ok)
				resolved_route = "egl-x11";
			else
				stats.last_error = "requested route unavailable: egl-x11";
		}
		else if (route == "glx-x11") {
			if (cap.glx_x11_ok)
				resolved_route = "glx-x11";
			else
				stats.last_error = "requested route unavailable: glx-x11";
		}
		else if (route == "stub-cpu") {
			stats.backend = "stub";
			stats.backend_route = "stub-cpu";
		}
		else {
			stats.last_error = "unsupported route: " + route;
		}
		if (stats.backend == "egl-gl" && !resolved_route.IsEmpty()) {
			if (resolved_route == "glx-x11") {
				backend.Create<GlxX11Backend>();
				String error;
				if (backend->Initialize(cfg, error)) {
					stats.backend_route = backend->GetRoute();
					stats.renderer = backend->GetRenderer();
				}
				else {
					stats.backend_route = "glx-x11-init-failed";
					stats.last_error = error;
					backend.Clear();
					stats.backend = "stub";
				}
			}
			else if (resolved_route == "egl-surfaceless") {
				backend.Create<EglSurfacelessBackend>();
				String error;
				if (backend->Initialize(cfg, error)) {
					stats.backend_route = backend->GetRoute();
					stats.renderer = backend->GetRenderer();
				}
				else {
					stats.backend_route = "egl-surfaceless-init-failed";
					stats.last_error = error;
					backend.Clear();
					stats.backend = "stub";
				}
			}
			else if (resolved_route == "egl-x11") {
				backend.Create<EglX11Backend>();
				String error;
				if (backend->Initialize(cfg, error)) {
					stats.backend_route = backend->GetRoute();
					stats.renderer = backend->GetRenderer();
				}
				else {
					stats.backend_route = "egl-x11-init-failed";
					stats.last_error = error;
					backend.Clear();
					stats.backend = "stub";
				}
			}
			else {
				stats.backend_route = resolved_route + "-not-yet-implemented";
				stats.last_error = "Backend route " + resolved_route + " not yet implemented";
				stats.backend = "stub";
			}
		}
		else if (stats.backend == "egl-gl") {
			stats.backend_route = "route-selection-failed";
			stats.backend = "stub";
		}
#else
		stats.backend = "stub";
		stats.backend_route = "stub-fallback";
		stats.last_error = "EGL backend unsupported on this platform";
#endif
	}
	stats.transfer_model = (stats.backend == "stub" ? "cpu-only" : "single-upload-compact-readback");
	if (stats.backend != "stub")
		cfg.compact_readback_only = true;
	stats.initialized = true;
	initialized = true;
	
	if (cfg.backend != "auto" && stats.backend == "stub" && cfg.backend != "stub")
		return false;
	
	return true;
}

void GpuPreprocessEngine::Shutdown() {
	if (backend) {
		backend->Shutdown();
		backend.Clear();
	}
	initialized = false;
	gray = ByteMat();
	smooth = ByteMat();
	pyramid_gray.Clear();
	pyramid_smooth.Clear();
	stats = GpuPreprocessStats();
}

bool GpuPreprocessEngine::IsAvailable() const {
	return initialized;
}

bool GpuPreprocessEngine::PrepareFrame(const Image& img) {
	stats.frame_prepared = false;
	stats.width = 0;
	stats.height = 0;
	stats.total_us = 0;
	stats.upload_us = 0;
	stats.grayscale_us = 0;
	stats.blur_us = 0;
	stats.pyramid_us = 0;
	stats.scoremap_us = 0;
	stats.nms_us = 0;
	stats.readback_us = 0;
	stats.upload_copies = 0;
	stats.readback_copies = 0;
	stats.upload_bytes = 0;
	stats.readback_bytes = 0;
	if (!initialized) {
		stats.last_error = "GpuPreprocessEngine not initialized";
		return false;
	}
	if (img.IsEmpty()) {
		stats.last_error = "Empty frame";
		return false;
	}
	TimeStop total_ts;
	stats.width = img.GetWidth();
	stats.height = img.GetHeight();

	if (backend && backend->IsReady()) {
		String gpu_error;
		if (backend->PrepareFrameGpu(img, cfg.pyramid_levels, stats, gpu_error)) {
			if (cfg.compact_readback_only) {
				// We still need the base grayscale image for ORB descriptor extraction
				TimeStop readback_ts;
				ByteMat base_gray;
				if (backend->GetGrayscaleGpu(base_gray, gpu_error)) {
					pyramid_gray.Clear();
					pyramid_gray.Add(pick(base_gray));
					gray = pyramid_gray[0];
					stats.readback_us = readback_ts.Elapsed();
					stats.readback_copies = 1;
					stats.readback_bytes = (int64)gray.cols * (int64)gray.rows;
				} else {
					stats.last_error = "GPU grayscale readback failed: " + gpu_error;
				}
			} else {
				TimeStop readback_ts;
				if (!backend->GetPyramidGrayGpu(pyramid_gray, gpu_error)) {
					stats.last_error = "GPU pyramid gray readback failed: " + gpu_error;
					pyramid_gray.Clear();
					pyramid_smooth.Clear();
				}
				else if (!backend->GetPyramidSmoothGpu(pyramid_smooth, gpu_error)) {
					stats.last_error = "GPU pyramid smooth readback failed: " + gpu_error;
					pyramid_smooth.Clear();
				}
				else {
					if (!pyramid_gray.IsEmpty()) gray = pyramid_gray[0];
					if (!pyramid_smooth.IsEmpty()) smooth = pyramid_smooth[0];
				}
				stats.readback_us = readback_ts.Elapsed();
				int64 readback_pixels = 0;
				for (const ByteMat& m : pyramid_gray)
					readback_pixels += (int64)m.cols * (int64)m.rows;
				for (const ByteMat& m : pyramid_smooth)
					readback_pixels += (int64)m.cols * (int64)m.rows;
				stats.readback_copies = pyramid_gray.GetCount() + pyramid_smooth.GetCount();
				stats.readback_bytes = readback_pixels;
			}
			stats.total_us = total_ts.Elapsed();
			stats.frame_prepared = true;
			return true;
		}
		stats.last_error = "GPU PrepareFrame failed: " + gpu_error + "; falling back to CPU";
	}

	ByteMat rgba(img.GetWidth(), img.GetHeight(), 4);
	{
		TimeStop ts;
		memcpy(rgba.data.Begin(), img.Begin(), img.GetLength() * sizeof(RGBA));
		stats.upload_us = ts.Elapsed();
		stats.upload_copies = 1;
		stats.upload_bytes = (int64)img.GetLength() * (int64)sizeof(RGBA);
	}
	{
		TimeStop ts;
		Grayscale(rgba, gray);
		stats.grayscale_us = ts.Elapsed();
	}
	{
		TimeStop ts;
		smooth.SetSize(gray.cols, gray.rows, 1);
		GaussianBlur(gray, smooth, 9);
		stats.blur_us = ts.Elapsed();
	}
	{
		TimeStop ts;
		BuildCpuPyramid(gray, smooth, cfg.pyramid_levels, pyramid_gray, pyramid_smooth);
		stats.pyramid_levels = pyramid_gray.GetCount();
		stats.pyramid_us = ts.Elapsed();
	}
	stats.total_us = total_ts.Elapsed();
	stats.frame_prepared = true;
	return true;
}

// Phase 12 Stage 2: Zero-copy frame support
bool GpuPreprocessEngine::PrepareFrame(const GpuFrame& frame) {
	if (frame.type == GpuFrame::CPU_IMAGE) {
		// Existing CPU path
		bool ok = PrepareFrame(frame.cpu_image);
		if (ok && cfg.compact_readback_only) {
			// Ensure pyramid_gray[0] exists for compact mode consistency
			if (pyramid_gray.IsEmpty()) pyramid_gray.Add(gray);
		}
		return ok;
	}
	#if defined(PLATFORM_POSIX) && !defined(PLATFORM_OSX)
	else if (frame.type == GpuFrame::DMA_BUF) {
		// Linux zero-copy path (Phase 12 Stage 3)
		stats.frame_prepared = false;
		stats.width = 0;
		stats.height = 0;
		stats.total_us = 0;
		stats.upload_us = 0;
		stats.grayscale_us = 0;
		stats.blur_us = 0;
		stats.pyramid_us = 0;
		stats.scoremap_us = 0;
		stats.nms_us = 0;
		stats.readback_us = 0;
		stats.upload_copies = 0;
		stats.readback_copies = 0;
		stats.upload_bytes = 0;
		stats.readback_bytes = 0;

		if (!initialized) {
			stats.last_error = "GpuPreprocessEngine not initialized";
			return false;
		}
		if (frame.dmabuf.fd < 0 || frame.dmabuf.width <= 0 || frame.dmabuf.height <= 0) {
			stats.last_error = "Invalid DMA-BUF descriptor";
			return false;
		}
		TimeStop total_ts;
		stats.width = frame.dmabuf.width;
		stats.height = frame.dmabuf.height;

		if (backend && backend->IsReady()) {
			String gpu_error;
			if (backend->PrepareFrameDmaBuf(frame.dmabuf, cfg.pyramid_levels, stats, gpu_error)) {
				// Same readback logic as CPU path
				if (cfg.compact_readback_only) {
					// ZERO readback in compact mode!
					// We only read back keypoints/descriptors/areas on demand.
					pyramid_gray.Clear();
					pyramid_smooth.Clear();
					gray = ByteMat();
					smooth = ByteMat();
				} else {
					TimeStop readback_ts;
					if (!backend->GetPyramidGrayGpu(pyramid_gray, gpu_error)) {
						stats.last_error = "GPU pyramid gray readback failed: " + gpu_error;
						pyramid_gray.Clear();
						pyramid_smooth.Clear();
					}
					else if (!backend->GetPyramidSmoothGpu(pyramid_smooth, gpu_error)) {
						stats.last_error = "GPU pyramid smooth readback failed: " + gpu_error;
						pyramid_smooth.Clear();
					}
					else {
						if (!pyramid_gray.IsEmpty()) gray = pyramid_gray[0];
						if (!pyramid_smooth.IsEmpty()) smooth = pyramid_smooth[0];
					}
					stats.readback_us = readback_ts.Elapsed();
					int64 readback_pixels = 0;
					for (const ByteMat& m : pyramid_gray)
						readback_pixels += (int64)m.cols * (int64)m.rows;
					for (const ByteMat& m : pyramid_smooth)
						readback_pixels += (int64)m.cols * (int64)m.rows;
					stats.readback_copies = pyramid_gray.GetCount() + pyramid_smooth.GetCount();
					stats.readback_bytes = readback_pixels;
				}
				stats.total_us = total_ts.Elapsed();
				stats.frame_prepared = true;
				return true;
			}
			stats.last_error = "GPU PrepareFrameDmaBuf failed: " + gpu_error;
			return false;
		}
		stats.last_error = "Backend not ready for DMA-BUF import";
		return false;
	}
	#endif
	#ifdef PLATFORM_WIN32
	else if (frame.type == GpuFrame::D3D11_TEXTURE) {
		// Windows zero-copy path (Phase 12 Task 2)
		stats.last_error = "D3D11 zero-copy path not yet implemented (Task 2)";
		return false;
	}
	#endif
	else if (frame.type == GpuFrame::GL_TEX) {
		// GL texture path (future)
		stats.last_error = "GL texture zero-copy path not yet implemented";
		return false;
	}
	stats.last_error = "Unknown GpuFrame type";
	return false;
}

bool GpuPreprocessEngine::ComputeScoreMaps() {
	if (!stats.frame_prepared) {
		stats.last_error = "ComputeScoreMaps: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		String error;
		if (backend->ComputeScoreMapsGpu(stats, error))
			return true;
		stats.last_error = "ComputeScoreMapsGpu failed: " + error;
		return false;
	}
	stats.last_error = "ComputeScoreMaps: backend not ready (current backend: " + stats.backend + ")";
	return false;
}

bool GpuPreprocessEngine::GetKeypoints(Vector<GpuKp>& out, int max_keypoints) {
	if (!stats.frame_prepared) {
		stats.last_error = "GetKeypoints: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		String error;
		TimeStop readback_ts;
		if (backend->GetKeypointsGpu(out, max_keypoints, error)) {
			self->stats.readback_us += readback_ts.Elapsed();
			self->stats.readback_copies++;
			int pw = stats.width, ph = stats.height;
			int64 bytes = 0;
			for(int i=0; i<cfg.pyramid_levels; i++) {
				bytes += (int64)pw * (int64)ph;
				pw = max(1, pw/2); ph = max(1, ph/2);
			}
			self->stats.readback_bytes += bytes;
			return true;
		}
		stats.last_error = "GetKeypointsGpu failed: " + error;
		return false;
	}
	stats.last_error = "GetKeypoints: backend not ready";
	return false;
}

bool GpuPreprocessEngine::ComputeDescriptors(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors) {
	if (!stats.frame_prepared) {
		stats.last_error = "ComputeDescriptors: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		String error;
		TimeStop describe_ts;
		if (backend->ComputeDescriptorsGpu(kps, descriptors, error)) {
			self->stats.describe_us = describe_ts.Elapsed();
			self->stats.readback_us += self->stats.describe_us; // Add to total readback for now
			self->stats.readback_copies++;
			self->stats.readback_bytes += (int64)descriptors.GetCount() * (int64)sizeof(BinDescriptor);
			return true;
		}
		self->stats.last_error = "ComputeDescriptorsGpu failed: " + error;
		return false;
	}
	const_cast<GpuPreprocessEngine*>(this)->stats.last_error = "ComputeDescriptors: backend not ready";
	return false;
}

bool GpuPreprocessEngine::ExtractSparsePatches(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches) {
	if (!stats.frame_prepared) {
		stats.last_error = "ExtractSparsePatches: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		String error;
		TimeStop readback_ts;
		if (backend->ExtractSparsePatchesGpu(kps, patches, error)) {
			stats.readback_us += readback_ts.Elapsed();
			stats.readback_copies++;
			stats.readback_bytes += (int64)patches.GetCount() * (int64)sizeof(GpuPatch);
			return true;
		}
		stats.last_error = "ExtractSparsePatchesGpu failed: " + error;
		return false;
	}
	stats.last_error = "ExtractSparsePatches: backend not ready";
	return false;
}

bool GpuPreprocessEngine::ReadbackAreas(const Vector<Rect>& rects, Vector<ByteMat>& outcomes) {
	if (!stats.frame_prepared) {
		stats.last_error = "ReadbackAreas: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		String error;
		TimeStop readback_ts;
		if (backend->ReadbackAreasGpu(rects, outcomes, error)) {
			self->stats.readback_us += readback_ts.Elapsed();
			self->stats.readback_copies++; // Single batched call (mostly)
			for (const auto& m : outcomes)
				self->stats.readback_bytes += (int64)m.cols * (int64)m.rows;
			return true;
		}
		self->stats.last_error = "ReadbackAreasGpu failed: " + error;
	}

	// CPU Fallback
	if (gray.IsEmpty()) {
		const_cast<GpuPreprocessEngine*>(this)->stats.last_error = "ReadbackAreas: backend not ready and no grayscale frame available";
		return false;
	}
	
	outcomes.SetCount(rects.GetCount());
	for (int i = 0; i < rects.GetCount(); i++) {
		Rect r = rects[i];
		int rw = r.GetWidth();
		int rh = r.GetHeight();
		if (rw <= 0 || rh <= 0) {
			outcomes[i].SetSize(0, 0, 1);
			continue;
		}
		outcomes[i].SetSize(rw, rh, gray.channels);
		for (int y = 0; y < rh; y++) {
			int sy = r.top + y;
			if (sy < 0 || sy >= gray.rows) continue;
			for (int x = 0; x < rw; x++) {
				int sx = r.left + x;
				if (sx < 0 || sx >= gray.cols) continue;
				for (int c = 0; c < gray.channels; c++) {
					outcomes[i].data[(y * rw + x) * gray.channels + c] = 
						gray.data[(sy * gray.cols + sx) * gray.channels + c];
				}
			}
		}
	}
	return true;
}

bool GpuPreprocessEngine::ReadbackBinarizedAreas(const Vector<Rect>& rects, float threshold, Vector<ByteMat>& outcomes) {
	if (!stats.frame_prepared) {
		stats.last_error = "ReadbackBinarizedAreas: frame not prepared";
		return false;
	}
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		String error;
		TimeStop readback_ts;
		if (backend->ReadbackBinarizedAreasGpu(rects, threshold, outcomes, error)) {
			self->stats.readback_us += readback_ts.Elapsed();
			self->stats.readback_copies++;
			for (const auto& m : outcomes)
				self->stats.readback_bytes += (int64)m.cols * (int64)m.rows;
			return true;
		}
		self->stats.last_error = "ReadbackBinarizedAreasGpu failed: " + error;
	}
	return false;
}

bool GpuPreprocessEngine::GetGray(ByteMat& out) const {
	if (!stats.frame_prepared) {
		const_cast<GpuPreprocessEngine*>(this)->stats.last_error = "GetGray: frame not prepared";
		return false;
	}
	if (!gray.IsEmpty()) {
		out = gray;
		return true;
	}
	if (!pyramid_gray.IsEmpty()) {
		out = pyramid_gray[0];
		return true;
	}
	
	// On-demand readback if in compact mode
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		GpuPreprocessBackend* b = const_cast<GpuPreprocessBackend*>(backend.Get());
		String error;
		TimeStop ts;
		if (b->GetGrayscaleGpu(self->gray, error)) {
			self->stats.readback_us += ts.Elapsed();
			self->stats.readback_copies++;
			self->stats.readback_bytes += (int64)self->gray.cols * (int64)self->gray.rows;
			out = self->gray;
			return true;
		}
		self->stats.last_error = "GetGray on-demand readback failed: " + error;
	}

	const_cast<GpuPreprocessEngine*>(this)->stats.last_error = "GetGray: no grayscale data available";
	return false;
}

bool GpuPreprocessEngine::GetSmooth(ByteMat& out) const {
	if (!stats.frame_prepared) return false;
	if (!smooth.IsEmpty()) {
		out = smooth;
		return true;
	}
	
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		GpuPreprocessBackend* b = const_cast<GpuPreprocessBackend*>(backend.Get());
		String error;
		TimeStop ts;
		if (b->GetSmoothGpu(self->smooth, error)) {
			self->stats.readback_us += ts.Elapsed();
			self->stats.readback_copies++;
			self->stats.readback_bytes += (int64)self->smooth.cols * (int64)self->smooth.rows;
			out = self->smooth;
			return true;
		}
		self->stats.last_error = "GetSmooth on-demand readback failed: " + error;
	}
	return false;
}

bool GpuPreprocessEngine::GetPyramidGray(Vector<ByteMat>& out) const {
	if (!stats.frame_prepared) return false;
	if (!pyramid_gray.IsEmpty() && (cfg.compact_readback_only ? pyramid_gray.GetCount() == 1 : pyramid_gray.GetCount() > 1)) {
		out = clone(pyramid_gray);
		return true;
	}
	
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		GpuPreprocessBackend* b = const_cast<GpuPreprocessBackend*>(backend.Get());
		String error;
		TimeStop ts;
		if (b->GetPyramidGrayGpu(self->pyramid_gray, error)) {
			self->stats.readback_us += ts.Elapsed();
			int64 bytes = 0;
			for (const auto& m : self->pyramid_gray) bytes += (int64)m.cols * (int64)m.rows;
			self->stats.readback_copies += self->pyramid_gray.GetCount();
			self->stats.readback_bytes += bytes;
			out = clone(self->pyramid_gray);
			return true;
		}
		self->stats.last_error = "GetPyramidGray on-demand readback failed: " + error;
	}
	return false;
}

bool GpuPreprocessEngine::GetPyramidSmooth(Vector<ByteMat>& out) const {
	if (!stats.frame_prepared) return false;
	if (!pyramid_smooth.IsEmpty()) {
		out = clone(pyramid_smooth);
		return true;
	}
	
	if (backend && backend->IsReady()) {
		GpuPreprocessEngine* self = const_cast<GpuPreprocessEngine*>(this);
		GpuPreprocessBackend* b = const_cast<GpuPreprocessBackend*>(backend.Get());
		String error;
		TimeStop ts;
		if (b->GetPyramidSmoothGpu(self->pyramid_smooth, error)) {
			self->stats.readback_us += ts.Elapsed();
			int64 bytes = 0;
			for (const auto& m : self->pyramid_smooth) bytes += (int64)m.cols * (int64)m.rows;
			self->stats.readback_copies += self->pyramid_smooth.GetCount();
			self->stats.readback_bytes += bytes;
			out = clone(self->pyramid_smooth);
			return true;
		}
		self->stats.last_error = "GetPyramidSmooth on-demand readback failed: " + error;
	}
	return false;
}

}
