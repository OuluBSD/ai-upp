#ifndef _GameCommon_GpuPreprocess_h_
#define _GameCommon_GpuPreprocess_h_

#include <ComputerVision/ComputerVision.h>

namespace Upp {

struct GpuPreprocessConfig : Moveable<GpuPreprocessConfig> {
	String backend = "stub";
	String route = "auto";
	bool headless = true;
	bool compact_readback_only = true;
	bool reuse_textures = true;
	bool reuse_pyramid = true;
	int max_width = 1920;
	int max_height = 1080;
	int pyramid_levels = 3;
};

struct GpuPreprocessStats : Moveable<GpuPreprocessStats> {
	bool initialized = false;
	bool frame_prepared = false;
	String requested_backend = "auto";
	String requested_route = "auto";
	String backend = "stub";
	String backend_route = "stub-cpu";
	String renderer;
	String last_error;
	String transfer_model = "cpu-only";
	int width = 0;
	int height = 0;
	int pyramid_levels = 0;
	int upload_copies = 0;
	int readback_copies = 0;
	int64 upload_bytes = 0;
	int64 readback_bytes = 0;
	int64 total_us = 0;
	int64 upload_us = 0;
	int64 grayscale_us = 0;
	int64 blur_us = 0;
	int64 pyramid_us = 0;
	int64 scoremap_us = 0;
	int64 nms_us = 0;
	int64 describe_us = 0;
	int64 readback_us = 0;
	int64 texture_reuse_count = 0;
	int64 texture_realloc_count = 0;
};

struct GpuBackendProbe : Moveable<GpuBackendProbe> {
	bool has_eglinfo = false;
	bool has_glxinfo = false;
	bool egl_surfaceless_ok = false;
	bool egl_x11_ok = false;
	bool glx_x11_ok = false;
	String preferred_backend = "stub";
	String preferred_route = "stub-cpu";
	String preferred_renderer;
	String surfaceless_renderer;
	String x11_renderer;
	String glx_renderer;
	String probe_error;
};

// Phase 12: Zero-copy frame descriptor for cross-platform GPU texture sharing
struct GpuFrame : Moveable<GpuFrame> {
	enum Type { CPU_IMAGE, DMA_BUF, D3D11_TEXTURE, GL_TEX };
	enum ExternalFormat {
		EXTERNAL_UNKNOWN,
		EXTERNAL_RGBA8,
		EXTERNAL_BGRA8,
		EXTERNAL_YUYV422,
	};
	Type type = CPU_IMAGE;

	// CPU path (existing - both platforms)
	Image cpu_image;

	// Linux zero-copy path (Phase 12 Task 1)
	struct DmaBufDesc {
		int fd = -1;
		uint32_t fourcc = 0;  // DRM format code (e.g., DRM_FORMAT_ARGB8888)
		int width = 0;
		int height = 0;
		int stride = 0;
		uint64_t modifier = 0;  // Tiling/compression modifier
		ExternalFormat external_format = EXTERNAL_UNKNOWN;
	} dmabuf;

	// Windows zero-copy path (Phase 12 Task 2)
	struct D3D11TextureDesc {
		void* texture = nullptr;  // ID3D11Texture2D* (void* to avoid header dependency)
		int width = 0;
		int height = 0;
		uint32_t format = 0;  // DXGI_FORMAT
		ExternalFormat external_format = EXTERNAL_UNKNOWN;
	} d3d11_tex;

	// Zero-copy GL texture path (future)
	struct GlTextureDesc {
		uint32_t tex = 0;  // GLuint
		int width = 0;
		int height = 0;
	} gltex;

	GpuFrame() {}
	explicit GpuFrame(const Image& img) : type(CPU_IMAGE), cpu_image(img) {}
};

class GpuPreprocessBackend;

typedef OrbSystem::GpuKp GpuKp;

struct GpuPatch : Moveable<GpuPatch> {
	byte data[32 * 32]; // Max patch size (e.g. 31x31 for ORB)
	int  width, height;
};

class GpuPreprocessEngine {
	GpuPreprocessConfig cfg;
	GpuPreprocessStats stats;
	ByteMat gray;
	ByteMat smooth;
	Vector<ByteMat> pyramid_gray;
	Vector<ByteMat> pyramid_smooth;
	bool initialized = false;
	One<GpuPreprocessBackend> backend;

public:
	GpuPreprocessEngine();
	~GpuPreprocessEngine();
	static String NormalizeBackend(String s);
	static String ResolveBackend(String s);
	static String NormalizeRoute(String s);
	static Vector<String> GetRouteNames();
	static Vector<String> GetBackendNames();
	static GpuBackendProbe ProbeCapabilities(bool refresh = false);
	static String FormatProbeReport(const GpuBackendProbe& p);
	bool Initialize(const GpuPreprocessConfig& cfg);
	void Shutdown();
	bool IsAvailable() const;
	bool PrepareFrame(const Image& img);
	bool PrepareFrame(const GpuFrame& frame);  // Phase 12: Zero-copy frame support
	bool ComputeScoreMaps();
	bool GetKeypoints(Vector<GpuKp>& out, int max_keypoints = 16384); // Uses GpuKp
	bool ComputeDescriptors(const Vector<GpuKp>& kps, Vector<BinDescriptor>& descriptors); // New method
	bool ExtractSparsePatches(const Vector<GpuKp>& kps, Vector<GpuPatch>& patches); // New method
	bool ReadbackAreas(const Vector<Rect>& rects, Vector<ByteMat>& outcomes); // New method
	bool ReadbackBinarizedAreas(const Vector<Rect>& rects, float threshold, Vector<ByteMat>& outcomes); // New method
	bool GetGray(ByteMat& out) const;
	bool GetSmooth(ByteMat& out) const;
	bool GetPyramidGray(Vector<ByteMat>& out) const;
	bool GetPyramidSmooth(Vector<ByteMat>& out) const;
	void MakeCurrent();
	const GpuPreprocessStats& GetStats() const { return stats; }
	const String& GetLastError() const { return stats.last_error; }
};

}

#endif
