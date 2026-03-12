#include "ScriptIDE.h"
#include <Painter/Painter.h>

#if defined(flagLINUX) && !defined(flagX11)
#include <librsvg/rsvg.h>
#include <cairo.h>
#endif

#ifdef flagWIN32
#include <windows.h>
#include <d3d11_1.h>
#include <d2d1_3.h>
#include <d2d1_1helper.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <comdef.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;

namespace Upp {

struct D2DSVGContext {
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dContext;
	ComPtr<ID2D1Factory1> d2dFactory;
	ComPtr<ID2D1Device> d2dDevice;
	ComPtr<ID2D1DeviceContext5> d2d;

	bool Init() {
		if (d2d) return true;
		
		D3D_FEATURE_LEVEL fl;
		HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		                               D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
		                               D3D11_SDK_VERSION, &d3dDevice, &fl, &d3dContext);
		if (FAILED(hr)) return false;

		ComPtr<IDXGIDevice> dxgiDevice;
		if (FAILED(d3dDevice.As(&dxgiDevice))) return false;

		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory)))) return false;
		if (FAILED(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice))) return false;
		
		ComPtr<ID2D1DeviceContext> d2dContextBase;
		if (FAILED(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContextBase))) return false;
		if (FAILED(d2dContextBase.As(&d2d))) return false;

		return true;
	}
};

static D2DSVGContext& GetD2D() {
	static D2DSVGContext ctx;
	return ctx;
}

DEINITBLOCK {
	GetD2D().d2d.Reset();
	GetD2D().d2dDevice.Reset();
	GetD2D().d2dFactory.Reset();
	GetD2D().d3dContext.Reset();
	GetD2D().d3dDevice.Reset();
}

}
#endif

namespace Upp {

static Image LoadSVG(const String& path, int size)
{
	String svg_text = LoadFile(path);
	if(svg_text.IsEmpty()) return Image();

	int render_size = 128; // Higher res for better quality

#if defined(flagLINUX) && !defined(flagX11)
	GError *error = NULL;
	RsvgHandle *handle = rsvg_handle_new_from_data((const guint8 *)~svg_text, svg_text.GetLength(), &error);
	if(handle) {
		cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, render_size, render_size);
		cairo_t *cr = cairo_create(surface);
		
		RsvgRectangle viewport;
		viewport.x = 0; viewport.y = 0;
		viewport.width = render_size; viewport.height = render_size;
		
		rsvg_handle_render_document(handle, cr, &viewport, &error);
		cairo_surface_flush(surface);
		
		unsigned char *data = cairo_image_surface_get_data(surface);
		int stride = cairo_image_surface_get_stride(surface);
		
		ImageBuffer ib(render_size, render_size);
		for(int y = 0; y < render_size; y++) {
			RGBA *line = ib[y];
			RGBA *src = (RGBA *)(data + y * stride);
			for(int x = 0; x < render_size; x++) {
				// Cairo ARGB32 is BGRA in memory on little endian.
				// U++ RGBA is RGBA in memory (byte r, g, b, a).
				// So we must swap R and B.
				line[x].r = src[x].b;
				line[x].g = src[x].g;
				line[x].b = src[x].r;
				line[x].a = src[x].a;
			}
		}
		
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		g_object_unref(handle);
		
		return Rescale(Image(ib), size, size);
	}
#endif

#ifdef flagWIN32
	if(GetD2D().Init()) {
		D2DSVGContext& ctx = GetD2D();
		
		D3D11_TEXTURE2D_DESC td = {};
		td.Width = render_size;
		td.Height = render_size;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET;

		ComPtr<ID3D11Texture2D> texture;
		if (SUCCEEDED(ctx.d3dDevice->CreateTexture2D(&td, nullptr, &texture))) {
			ComPtr<IDXGISurface> surface;
			if (SUCCEEDED(texture.As(&surface))) {
				D2D1_BITMAP_PROPERTIES1 bp = D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
					96.0f, 96.0f);

				ComPtr<ID2D1Bitmap1> targetBitmap;
				if (SUCCEEDED(ctx.d2d->CreateBitmapFromDxgiSurface(surface.Get(), &bp, &targetBitmap))) {
					ctx.d2d->SetTarget(targetBitmap.Get());
					
					ComPtr<IStream> svgStream;
					size_t len = svg_text.GetLength();
					HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
					if (mem) {
						void* p = GlobalLock(mem);
						memcpy(p, ~svg_text, len);
						GlobalUnlock(mem);
						if (SUCCEEDED(CreateStreamOnHGlobal(mem, TRUE, &svgStream))) {
							ComPtr<ID2D1SvgDocument> svgDoc;
							if (SUCCEEDED(ctx.d2d->CreateSvgDocument(svgStream.Get(), D2D1::SizeF((float)render_size, (float)render_size), &svgDoc))) {
								ctx.d2d->BeginDraw();
								ctx.d2d->Clear(D2D1::ColorF(0, 0.0f));
								ctx.d2d->DrawSvgDocument(svgDoc.Get());
								if (SUCCEEDED(ctx.d2d->EndDraw())) {
									D3D11_TEXTURE2D_DESC sd = td;
									sd.BindFlags = 0;
									sd.Usage = D3D11_USAGE_STAGING;
									sd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
									ComPtr<ID3D11Texture2D> staging;
									if (SUCCEEDED(ctx.d3dDevice->CreateTexture2D(&sd, nullptr, &staging))) {
										ctx.d3dContext->CopyResource(staging.Get(), texture.Get());
										D3D11_MAPPED_SUBRESOURCE mapped = {};
										if (SUCCEEDED(ctx.d3dContext->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
											ImageBuffer ib(render_size, render_size);
											for(int y = 0; y < render_size; y++) {
												RGBA *line = ib[y];
												RGBA *src = (RGBA *)((byte*)mapped.pData + y * mapped.RowPitch);
												for(int x = 0; x < render_size; x++) {
													// DXGI_FORMAT_B8G8R8A8_UNORM is BGRA
													line[x].r = src[x].b;
													line[x].g = src[x].g;
													line[x].b = src[x].r;
													line[x].a = src[x].a;
												}
											}
											ctx.d3dContext->Unmap(staging.Get(), 0);
											return Rescale(Image(ib), size, size);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
#endif

	// Built-in U++ Painter fallback
	return RenderSVGImage(Size(size, size), svg_text, IsDarkTheme() ? White() : Black());
}

static Image LoadPNG(const String& path)
{
	return StreamRaster::LoadFileAny(path);
}

Image GetIcon(const char *name, int size)
{
	static VectorMap<String, Image> cache;
	
	String theme = IsDarkTheme() ? "dark" : "light";
	String key;
	key << theme << ":" << name << ":" << size;
	
	int f = cache.Find(key);
	if(f >= 0) return cache[f];
	
	Image img;
	String base_dir = "/common/active/sblo/Dev/ai-upp/share/icons/";
	
	// 1. Try Spyder theme-specific SVG
	String path;
	path << base_dir << theme << "/spyder/" << name << ".svg";
	if(FileExists(path)) img = LoadSVG(path, size);
	
	// 2. Try Spyder common SVG
	if(!img) {
		path = base_dir;
		path << "spyder/" << name << ".svg";
		if(FileExists(path)) img = LoadSVG(path, size);
	}
	
	// 3. Try Tabler theme-specific PNG (Fallback)
	if(!img) {
		path = base_dir;
		path << theme << "/tabler/outline/" << name << "_" << size << ".png";
		if(FileExists(path)) img = LoadPNG(path);
	}
	
	// 4. Try Tabler common PNG (Fallback)
	if(!img) {
		path = base_dir;
		path << "tabler/outline/" << name << "_" << size << ".png";
		if(FileExists(path)) img = LoadPNG(path);
	}
	
	// 5. Hardcoded fallbacks for specific names if still missing
	if(!img) {
		if(String(name) == "run_selection") return GetIcon("player-play", size);
		if(String(name) == "run_cell") return GetIcon("player-skip-forward", size);
		if(String(name) == "run_cell_advance") return GetIcon("player-track-next", size);
		if(String(name) == "debug") return GetIcon("bug", size);
		if(String(name) == "debug_cell") return GetIcon("bug-off", size);
		if(String(name) == "debug_selection") return GetIcon("microscope", size);
		if(String(name) == "profiler") return GetIcon("analyze", size);
		if(String(name) == "profile_cell") return GetIcon("adjustments-horizontal", size);
		if(String(name) == "profile_selection") return GetIcon("chart-dots", size);
		
		if(String(name) == "project_new") return GetIcon("file-plus", size);
		if(String(name) == "project_open") return GetIcon("folder-open", size);
		if(String(name) == "run_settings") return GetIcon("device-floppy", size);
		if(String(name) == "ArchiveFileIcon") return GetIcon("file-stack", size);
		if(String(name) == "project_close") return GetIcon("player-stop", size);
		if(String(name) == "findprevious") return GetIcon("arrow-back-up", size);
		if(String(name) == "findnext") return GetIcon("arrow-forward-up", size);
		if(String(name) == "undo") return GetIcon("arrow-back-up", size);
		if(String(name) == "redo") return GetIcon("arrow-forward-up", size);
		if(String(name) == "filesave") return GetIcon("device-floppy", size);
		if(String(name) == "save_all") return GetIcon("file-stack", size);
	}

	if(!img) {
		// Final fallback placeholder
		img = CtrlImg::help();
	}
	
	cache.Add(key, img);
	return img;
}

}
