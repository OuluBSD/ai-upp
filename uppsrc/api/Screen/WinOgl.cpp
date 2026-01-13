#include "Screen.h"

#if defined flagWIN32 && defined flagOGL && !defined flagUWP

#include <Core/Core.h>
#define CY win32_CY_
#define FAR win32_FAR_
#include <windows.h>
#undef CY
#undef FAR
#include <GL/gl.h>
#include <GL/glu.h>
#include <string.h>

// For OpenGL on Windows, we need to load OpenGL functions dynamically
// This is because the standard opengl32.dll only provides OpenGL 1.1 functions
// We'll need extensions for modern OpenGL features
#include <GL/glext.h>
#include <GL/wglext.h>

// Define function pointers for OpenGL extensions we might need
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = nullptr;

NAMESPACE_UPP

struct ScrWinOgl::NativeContext {
    HWND hwnd;
    HGLRC hrc;  // OpenGL rendering context
    HDC hdc;    // Device context
    bool running = false;
};

struct ScrWinOgl::NativeSinkDevice {
    NativeContext* ctx;
    GfxAccelAtom<WinOglGfx> accel;
    Size sz;
    bool vsync_enabled = false;
};

struct ScrWinOgl::NativeEventsBase {
    NativeContext* ctx;
    int time;
    dword seq;
    Vector<UPP::GeomEvent> ev;
    Size sz;
    bool ev_sendable;
    bool is_lalt;
    bool is_ralt;
    bool is_lshift;
    bool is_rshift;
    bool is_lctrl;
    bool is_rctrl;
    Point prev_mouse_pt;
};

// Helper function to load OpenGL extensions
bool Load_OpenGL_Extensions(ScrWinOgl::NativeContext& ctx) {
    // Create a dummy context to load extensions
    HGLRC tempContext = wglCreateContext(ctx.hdc);
    wglMakeCurrent(ctx.hdc, tempContext);
    
    // Load extension functions
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);
    
    return wglCreateContextAttribsARB != nullptr;
}

// Check if an OpenGL extension is supported
bool IsExtensionSupported(const char* ext) {
    if (wglGetExtensionsStringEXT != nullptr) {
        const char* extensions = wglGetExtensionsStringEXT();
        if (extensions) {
            const char* start = extensions;
            const char* where = nullptr;
            const char* terminator = nullptr;

            // Extension names should not have spaces.
            for (size_t i = 0; i < strlen(ext); i++) {
                if (ext[i] == ' ') return false;
            }

            if (ext[0] == '\0') return false;

            while ((where = strstr(start, ext)) != nullptr) {
                terminator = where + strlen(ext);
                if (where == start || *(where - 1) == ' ') {
                    if (*terminator == ' ' || *terminator == '\0') {
                        return true;
                    }
                }
                start = terminator;
            }
        }
    }
    return false;
}

GfxAccelAtom<WinOglGfx>& Get_ScrWinOgl_Ogl(ScrWinOgl::NativeSinkDevice*& dev) {
    return dev->accel;
}

bool ScrWinOgl::SinkDevice_Create(NativeSinkDevice*& dev) {
    dev = new NativeSinkDevice;
    return true;
}

void ScrWinOgl::SinkDevice_Destroy(NativeSinkDevice*& dev) {
    delete dev;
}

void ScrWinOgl::SinkDevice_Visit(NativeSinkDevice& dev, AtomBase&, Visitor& v) {
    v VISN(dev.accel);
}

LRESULT CALLBACK WinOgl_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Handle window messages for OpenGL window
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            // Handle window resize
            if (wParam != SIZE_MINIMIZED) {
                // Update viewport if needed
                if (wglGetCurrentContext() != NULL) {
                    int width = LOWORD(lParam);
                    int height = HIWORD(lParam);
                    glViewport(0, 0, width, height);
                }
            }
            break;
        case WM_PAINT:
            // For OpenGL, we handle rendering differently
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool ScrWinOgl::SinkDevice_Initialize(NativeSinkDevice& dev, AtomBase& a, const WorldState& ws) {
    auto ctx_ = a.val.FindOwnerWithCastDeep<WinOglContext>();
    if (!ctx_) {
        LOG("error: could not find WinOgl context");
        return false;
    }
    auto& ctx = *ctx_->dev;
    dev.ctx = &ctx;
    
    // Get window dimensions from WorldState
    int width = ws.GetInt(".width", 1280);
    int height = ws.GetInt(".height", 720);
    String title = ws.GetString(".title", "Topside OpenGL Window");
    bool fullscreen = ws.GetBool(".fullscreen", false);
    
    dev.sz.cx = width;
    dev.sz.cy = height;
    
    // Check if VSync should be enabled
    dev.vsync_enabled = ws.GetBool(".vsync", true);
    
    #if VIRTUALGUI
    HINSTANCE instance = 0;
    LOG("ScrWinOgl::SinkDevice_Initialize: error: can't access window instance with VIRTUALGUI");
    return false;
    #else
    HINSTANCE instance = AppGetHandle();
    if (!instance) {
        LOG("ScrWinOgl::SinkDevice_Initialize: error: no gui instance");
        return false;
    }
    #endif
    
    // Register window class for OpenGL window
    WNDCLASS wc = {};
    wc.lpfnWndProc = WinOgl_WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = "ScrWinOgl_Class";
    wc.style = CS_OWNDC; // Required for OpenGL to work properly
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClass(&wc);
    
    // Create the OpenGL window
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    if (fullscreen) {
        windowStyle = WS_POPUP | WS_VISIBLE;
    }
    
    ctx.hwnd = CreateWindowEx(
        0,                          // Extended window style
        wc.lpszClassName,           // Window class name
        title.Begin(),              // Window title
        windowStyle,                // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, // Position
        width, height,              // Size
        NULL,                       // Parent window
        NULL,                       // Menu
        instance,                   // Instance handle
        NULL                        // Additional application data
    );
    
    if (ctx.hwnd == NULL) {
        LOG("ScrWinOgl::SinkDevice_Initialize: error: could not create window");
        return false;
    }
    
    // Get the device context
    ctx.hdc = GetDC(ctx.hwnd);
    
    // Set up pixel format for OpenGL
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of pixel format to request
        32,                   // Color depth of the framebuffer
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(ctx.hdc, &pfd);
    SetPixelFormat(ctx.hdc, pixelFormat, &pfd);
    
    // Create temporary OpenGL context to load extensions
    HGLRC tempContext = wglCreateContext(ctx.hdc);
    wglMakeCurrent(ctx.hdc, tempContext);
    
    // Attempt to load OpenGL extensions
    Load_OpenGL_Extensions(ctx);
    
    // Create modern OpenGL context if possible
    HGLRC modernContext = NULL;
    
    // Try to create OpenGL 3.3 or 4.x context if extension is available
    if (wglCreateContextAttribsARB) {
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            // Removed WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB to allow deprecated functions
            0
        };
        
        modernContext = wglCreateContextAttribsARB(ctx.hdc, 0, attribs);
        if (!modernContext) {
            LOG("ScrWinOgl::SinkDevice_Initialize: warning: could not create modern OpenGL context, falling back to compatibility context");
        }
    }
    
    // Clean up temporary context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    
    // Use either the modern context or fall back to the old one
    if (modernContext) {
        ctx.hrc = modernContext;
    } else {
        ctx.hrc = wglCreateContext(ctx.hdc);
    }
    
    wglMakeCurrent(ctx.hdc, ctx.hrc);
    
    // Enable VSync if requested
    if (dev.vsync_enabled && wglSwapIntervalEXT) {
        wglSwapIntervalEXT(1);
    } else if (dev.vsync_enabled) {
        LOG("ScrWinOgl::SinkDevice_Initialize: warning: VSync extension not available");
    }
    
    // Set the viewport to match the window size
    glViewport(0, 0, width, height);
    
    // Initialize OpenGL state
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Show the window
    ShowWindow(ctx.hwnd, SW_SHOW);
    
    // Initialize GfxAccelAtom
    if (!dev.accel.Initialize(a, ws)) {
        LOG("ScrWinOgl::SinkDevice_Initialize: error: accelerator initialization failed");
        return false;
    }
    
    dev.accel.SetNative(ctx.hdc, ctx.hwnd, &ctx.hrc, 0);
    
    if (!dev.accel.Open(Size(width, height), 4, true)) {
        LOG("ScrWinOgl::SinkDevice_Initialize: error: could not open opengl atom");
        return false;
    }
    
    return true;
}

bool ScrWinOgl::SinkDevice_PostInitialize(NativeSinkDevice& dev, AtomBase& a) {
    return dev.accel.PostInitialize();
}

bool ScrWinOgl::SinkDevice_Start(NativeSinkDevice& dev, AtomBase& a) {
    return true;
}

void ScrWinOgl::SinkDevice_Stop(NativeSinkDevice& dev, AtomBase& a) {
    // Clean up OpenGL resources if needed
}

void ScrWinOgl::SinkDevice_Uninitialize(NativeSinkDevice& dev, AtomBase& a) {
    auto& ctx = *dev.ctx;
    
    dev.accel.Uninitialize();
    
    // Clean up OpenGL context
    if (ctx.hrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(ctx.hrc);
        ctx.hrc = NULL;
    }
    
    // Clean up device context
    if (ctx.hdc) {
        ReleaseDC(ctx.hwnd, ctx.hdc);
        ctx.hdc = NULL;
    }
    
    // Destroy window
    if (ctx.hwnd) {
        DestroyWindow(ctx.hwnd);
        ctx.hwnd = NULL;
    }
}

bool ScrWinOgl::SinkDevice_Recv(NativeSinkDevice& dev, AtomBase&, int ch_i, const Packet& p) {
    return dev.accel.Recv(ch_i, p);
}

void ScrWinOgl::SinkDevice_Finalize(NativeSinkDevice& dev, AtomBase&, RealtimeSourceConfig& cfg) {
    auto& ctx = *dev.ctx;
    
    if (!ctx.running)
        return;
    
    // Swap the OpenGL buffers to display the rendered frame
    if (ctx.hdc) {
        SwapBuffers(ctx.hdc);
    }
}

bool ScrWinOgl::SinkDevice_Send(NativeSinkDevice& dev, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
    dev.accel.Render(cfg);
    return true;
}

bool ScrWinOgl::SinkDevice_NegotiateSinkFormat(NativeSinkDevice& dev, AtomBase&, LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
    return false;
}

bool ScrWinOgl::SinkDevice_IsReady(NativeSinkDevice& dev, AtomBase&, PacketIO& io) {
    // Process Windows messages
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return true;
}

bool ScrWinOgl::Context_Create(NativeContext*& dev) {
    dev = new NativeContext;
    return true;
}

void ScrWinOgl::Context_Destroy(NativeContext*& dev) {
    delete dev;
}

void ScrWinOgl::Context_Visit(NativeContext& dev, AtomBase&, Visitor& vis) {
    
}

bool ScrWinOgl::Context_Initialize(NativeContext& ctx, AtomBase& a, const WorldState& ws) {
    ctx.running = true;
    return true;
}

bool ScrWinOgl::Context_PostInitialize(NativeContext& ctx, AtomBase& a) {
    return true;
}

bool ScrWinOgl::Context_Start(NativeContext& ctx, AtomBase& a) {
    return true;
}

void ScrWinOgl::Context_Stop(NativeContext& ctx, AtomBase& a) {
    
}

void ScrWinOgl::Context_Uninitialize(NativeContext& ctx, AtomBase& a) {
    
}

bool ScrWinOgl::Context_Send(NativeContext& ctx, AtomBase& a, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch) {
    return false;
}

bool ScrWinOgl::Context_Recv(NativeContext& ctx, AtomBase& a, int sink_ch, const Packet&) {
    return false;
}

void ScrWinOgl::Context_Finalize(NativeContext& ctx, AtomBase& a, RealtimeSourceConfig&) {
    
}

bool ScrWinOgl::Context_NegotiateSinkFormat(NativeContext& ctx, AtomBase& a, LinkBase& link, int sink_ch, const ValueFormat& new_fmt) {
    return false;
}

bool ScrWinOgl::Context_IsReady(NativeContext& dev, AtomBase&, PacketIO& io) {
    return true;
}

#define ABBR Ogl
#define WINIMPL 1
#include "Impl.inl"
#undef ABBR

END_UPP_NAMESPACE

#endif