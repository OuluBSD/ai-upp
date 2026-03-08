#include "DxAmpGraphics.h"

#ifndef _SILENCE_AMP_DEPRECATION_WARNINGS
#define _SILENCE_AMP_DEPRECATION_WARNINGS
#endif

#ifndef COUTLOG
#define COUTLOG(x) do { } while (0)
#define DXAMPGRAPHICS_UNDEF_COUTLOG
#endif

using namespace Upp;
#include <AMP/AMP.h>

#ifdef DXAMPGRAPHICS_UNDEF_COUTLOG
#undef COUTLOG
#undef DXAMPGRAPHICS_UNDEF_COUTLOG
#endif

#include <cmath>
#include <cstdint>
#include <cstring>

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#endif

#if defined(flagWIN32) && (defined(flagDX11) || defined(flagDX12))
#define WIN32_LEAN_AND_MEAN
typedef win32_CY_ CY;
#include <dxgi1_6.h>

#ifdef flagDX11
#include <d3d11.h>
#endif

#ifdef flagDX12
#include <d3d12.h>
#endif
#endif

using namespace Upp;
using namespace concurrency;

namespace {

struct MandelbrotGenerator {
	int width = 1280;
	int height = 720;
	int max_iter = 256;
	Vector<uint32> pixels;

	MandelbrotGenerator(int w, int h) : width(w), height(h) {
		pixels.SetCount(width * height);
	}

	void Render(float t) {
		array_view<uint32, 2> img(height, width, pixels.Begin());
		const float invw = 1.0f / (float)width;
		const float invh = 1.0f / (float)height;
		const float scale = 1.0f + 0.25f * (float)std::sin((double)t * 0.35);
		const float cx = -0.745f + 0.07f * (float)std::cos((double)t * 0.17);
		const float cy = 0.120f + 0.07f * (float)std::sin((double)t * 0.21);
		const int iter_limit = max_iter;

		parallel_for_each(img.extent, [=](index<2> idx) PARALLEL {
			float px = ((float)idx[1] * invw - 0.5f) * 3.2f / scale + cx;
			float py = ((float)idx[0] * invh - 0.5f) * 2.2f / scale + cy;
			float x = 0.0f;
			float y = 0.0f;
			int iter = 0;
			while (x * x + y * y <= 4.0f && iter < iter_limit) {
				float nx = x * x - y * y + px;
				y = 2.0f * x * y + py;
				x = nx;
				iter++;
			}

			uint32 color;
			if (iter >= iter_limit) {
				color = 0xFF000000u;
			}
			else {
				const uint32 c = (uint32)((iter * 255) / iter_limit);
				const uint32 r = c;
				const uint32 g = (c * 5u) >> 3;
				const uint32 b = 255u - c;
				color = 0xFF000000u | (r << 16) | (g << 8) | b;
			}
			img[idx] = color;
		});

		img.synchronize();
	}
};

void SavePpm(const char* path, const Vector<uint32>& pixels, int w, int h) {
	FileOut out(path);
	if (!out.IsOpen())
		return;
	out.Put("P6\n" + AsString(w) + " " + AsString(h) + "\n255\n");
	Buffer<byte> row(w * 3);
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uint32 c = pixels[y * w + x];
			row[x * 3 + 0] = (byte)((c >> 16) & 0xFF);
			row[x * 3 + 1] = (byte)((c >> 8) & 0xFF);
			row[x * 3 + 2] = (byte)(c & 0xFF);
		}
		out.Put(row, w * 3);
	}
}

#if defined(flagWIN32) && (defined(flagDX11) || defined(flagDX12))

template <class T>
struct ComPtr {
	T* ptr = nullptr;

	~ComPtr() {
		if (ptr)
			ptr->Release();
	}

	T* operator->() const { return ptr; }
	operator T*() const { return ptr; }
	T** operator&() {
		if (ptr) {
			ptr->Release();
			ptr = nullptr;
		}
		return &ptr;
	}
	T* Get() const { return ptr; }
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

HWND CreateRenderWindow(HINSTANCE hinst, int w, int h, const wchar_t* title) {
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hinst;
	wc.lpszClassName = L"DxAmpGraphicsWindow";
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClassExW(&wc);

	RECT rc = {0, 0, w, h};
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, nullptr, hinst, nullptr);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);
	return hwnd;
}

#ifdef flagDX11

class Dx11Viewer {
public:
	bool Init(HWND wnd, int w, int h) {
		hwnd = wnd;
		width = w;
		height = h;

		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT flags = 0;
		D3D_FEATURE_LEVEL level;
		D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
			levels, 2, D3D11_SDK_VERSION, &sd, &swapchain, &device, &level, &context);
		if (FAILED(hr))
			return false;

		hr = swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
		if (FAILED(hr))
			return false;

		D3D11_TEXTURE2D_DESC td = {};
		td.Width = width;
		td.Height = height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = 0;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		hr = device->CreateTexture2D(&td, nullptr, &frame_tex);
		return SUCCEEDED(hr);
	}

	void Draw(const Vector<uint32>& pixels) {
		const UINT pitch = (UINT)(width * sizeof(uint32));
		context->UpdateSubresource(frame_tex, 0, nullptr, pixels.Begin(), pitch, 0);
		context->CopyResource(backbuffer, frame_tex);
		swapchain->Present(1, 0);
	}

private:
	HWND hwnd = nullptr;
	int width = 0;
	int height = 0;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapchain;
	ComPtr<ID3D11Texture2D> backbuffer;
	ComPtr<ID3D11Texture2D> frame_tex;
};

#endif

#ifdef flagDX12

class Dx12Viewer {
public:
	static const UINT FrameCount = 2;

	bool Init(HWND wnd, int w, int h) {
		hwnd = wnd;
		width = w;
		height = h;

		if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
			return false;

		D3D12_COMMAND_QUEUE_DESC qdesc = {};
		qdesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		if (FAILED(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue))))
			return false;

		ComPtr<IDXGIFactory4> factory;
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
			return false;

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.BufferCount = FrameCount;
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> sc1;
		if (FAILED(factory->CreateSwapChainForHwnd(queue, hwnd, &sd, nullptr, nullptr, &sc1)))
			return false;
		if (FAILED(sc1->QueryInterface(IID_PPV_ARGS(&swapchain))))
			return false;
		frame_index = swapchain->GetCurrentBackBufferIndex();

		D3D12_DESCRIPTOR_HEAP_DESC rh = {};
		rh.NumDescriptors = FrameCount;
		rh.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		if (FAILED(device->CreateDescriptorHeap(&rh, IID_PPV_ARGS(&rtv_heap))))
			return false;
		rtv_stride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		if (!InitFrames())
			return false;

		if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame[0].alloc, nullptr, IID_PPV_ARGS(&list))))
			return false;
		list->Close();

		if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
			return false;
		fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		return fence_event != nullptr;
	}

	~Dx12Viewer() {
		if (fence_event)
			CloseHandle(fence_event);
	}

	void Draw(const Vector<uint32>& pixels) {
		frame_index = swapchain->GetCurrentBackBufferIndex();
		FrameRes& fr = frame[frame_index];
		WaitForFrame(fr);

		const UINT row_pitch = Align((UINT)(width * sizeof(uint32)), (UINT)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		for (int y = 0; y < height; ++y) {
			const byte* src = (const byte*)pixels.Begin() + y * width * sizeof(uint32);
			byte* dst = fr.upload_map + y * row_pitch;
			memcpy(dst, src, width * sizeof(uint32));
		}

		fr.alloc->Reset();
		list->Reset(fr.alloc, nullptr);

		D3D12_RESOURCE_BARRIER begin = {};
		begin.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		begin.Transition.pResource = fr.backbuffer;
		begin.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		begin.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		begin.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->ResourceBarrier(1, &begin);

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = fr.backbuffer;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = fr.upload;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		src.PlacedFootprint.Footprint.Width = width;
		src.PlacedFootprint.Footprint.Height = height;
		src.PlacedFootprint.Footprint.Depth = 1;
		src.PlacedFootprint.Footprint.RowPitch = row_pitch;

		list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		D3D12_RESOURCE_BARRIER end = {};
		end.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		end.Transition.pResource = fr.backbuffer;
		end.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		end.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		end.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->ResourceBarrier(1, &end);

		list->Close();
		ID3D12CommandList* work[] = {list};
		queue->ExecuteCommandLists(1, work);
		swapchain->Present(1, 0);

		fr.fence_value = ++fence_value;
		queue->Signal(fence, fr.fence_value);
	}

private:
	struct FrameRes {
		ComPtr<ID3D12Resource> backbuffer;
		ComPtr<ID3D12CommandAllocator> alloc;
		ComPtr<ID3D12Resource> upload;
		byte* upload_map = nullptr;
		UINT64 fence_value = 0;
	};

	HWND hwnd = nullptr;
	int width = 0;
	int height = 0;
	UINT frame_index = 0;
	UINT rtv_stride = 0;
	UINT64 fence_value = 0;
	HANDLE fence_event = nullptr;

	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> queue;
	ComPtr<IDXGISwapChain3> swapchain;
	ComPtr<ID3D12DescriptorHeap> rtv_heap;
	ComPtr<ID3D12GraphicsCommandList> list;
	ComPtr<ID3D12Fence> fence;
	FrameRes frame[FrameCount];

	static UINT Align(UINT value, UINT alignment) {
		return (value + alignment - 1u) & ~(alignment - 1u);
	}

	bool InitFrames() {
		D3D12_CPU_DESCRIPTOR_HANDLE h = rtv_heap->GetCPUDescriptorHandleForHeapStart();
		const UINT row_pitch = Align((UINT)(width * sizeof(uint32)), (UINT)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		const UINT64 upload_size = (UINT64)row_pitch * (UINT64)height;

		for (UINT i = 0; i < FrameCount; ++i) {
			if (FAILED(swapchain->GetBuffer(i, IID_PPV_ARGS(&frame[i].backbuffer))))
				return false;
			device->CreateRenderTargetView(frame[i].backbuffer, nullptr, h);
			h.ptr += rtv_stride;

			if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame[i].alloc))))
				return false;

			D3D12_HEAP_PROPERTIES hp = {};
			hp.Type = D3D12_HEAP_TYPE_UPLOAD;
			D3D12_RESOURCE_DESC rd = {};
			rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rd.Width = upload_size;
			rd.Height = 1;
			rd.DepthOrArraySize = 1;
			rd.MipLevels = 1;
			rd.SampleDesc.Count = 1;
			rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			if (FAILED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&frame[i].upload))))
				return false;

			if (FAILED(frame[i].upload->Map(0, nullptr, (void**)&frame[i].upload_map)))
				return false;
		}
		return true;
	}

	void WaitForFrame(FrameRes& fr) {
		if (fr.fence_value == 0)
			return;
		if (fence->GetCompletedValue() >= fr.fence_value)
			return;
		fence->SetEventOnCompletion(fr.fence_value, fence_event);
		WaitForSingleObject(fence_event, INFINITE);
	}
};

#endif

#endif

}

#ifndef flagGUI
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	MandelbrotGenerator gen(1280, 720);
	RLOG("DxAmpGraphics starting");
	RLOG(GetAmpDevices());

#if defined(flagWIN32) && (defined(flagDX11) || defined(flagDX12))
	HINSTANCE hinst = GetModuleHandleW(nullptr);
	HWND hwnd = CreateRenderWindow(hinst, gen.width, gen.height,
		#ifdef flagDX12
			L"DxAmpGraphics (DX12 + AMP Mandelbrot)"
		#else
			L"DxAmpGraphics (DX11 + AMP Mandelbrot)"
		#endif
	);
	if (!hwnd) {
		RLOG("Failed to create window");
		return;
	}

	#ifdef flagDX12
	Dx12Viewer viewer;
	#else
	Dx11Viewer viewer;
	#endif

	if (!viewer.Init(hwnd, gen.width, gen.height)) {
		RLOG("Failed to initialize DirectX renderer");
		return;
	}

	TimeStop ts;
	MSG msg;
	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				return;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		gen.Render((float)ts.Seconds());
		viewer.Draw(gen.pixels);
	}
#else
	gen.Render(0.0f);
	SavePpm("mandelbrot.ppm", gen.pixels, gen.width, gen.height);
	RLOG("Saved mandelbrot.ppm (non-DX build)");
#endif
}

#else
class DxAmpGraphicsWindow : public TopWindow {
public:
	DxAmpGraphicsWindow() : gen(1280, 720) {
		Title("DxAmpGraphics (U++ GUI + AMPCOMPAT)");
		Sizeable().Zoomable();
		SetRect(0, 0, 1280, 720);
		BackPaint();
	}

	void Paint(Draw& w) override {
		Size sz = GetSize();
		if (frame.IsEmpty()) {
			w.DrawRect(sz, Black());
			return;
		}
		w.DrawImage(0, 0, sz.cx, sz.cy, frame);
	}

	void Start() {
		UpdateFrame();
		SetTimeCallback(16, callback(this, &DxAmpGraphicsWindow::Tick), 1);
	}

private:
	MandelbrotGenerator gen;
	Image frame;
	TimeStop time;

	void Tick() {
		UpdateFrame();
		Refresh();
		SetTimeCallback(16, callback(this, &DxAmpGraphicsWindow::Tick), 1);
	}

	void UpdateFrame() {
		gen.Render((float)time.Seconds());
		ImageBuffer ib(gen.width, gen.height);
		RGBA* dst = ib.Begin();
		const uint32* src = gen.pixels.Begin();
		int n = gen.width * gen.height;
		for (int i = 0; i < n; ++i) {
			uint32 c = src[i];
			dst[i].r = (byte)((c >> 16) & 0xFF);
			dst[i].g = (byte)((c >> 8) & 0xFF);
			dst[i].b = (byte)(c & 0xFF);
			dst[i].a = 255;
		}
		frame = ib;
	}
};

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	RLOG("DxAmpGraphics GUI starting");
	RLOG(GetAmpDevices());

	DxAmpGraphicsWindow app;
	app.Start();
	app.Run();
}
#endif
