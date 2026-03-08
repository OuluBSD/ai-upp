#ifdef PLATFORM_WIN32
#define _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wrl/client.h>
#endif

#include <Core/Core.h>
#include <EditorCommon/Capture.h>

namespace Upp {

#ifdef PLATFORM_WIN32
using Microsoft::WRL::ComPtr;

// --- Windows Media Foundation Capture ---

class WinMFCapture : public CaptureSource {
	ComPtr<IMFSourceReader> reader;
	Size size;
	String name;
	bool opened = false;

public:
	WinMFCapture() {
		size = Size(1920, 1080);
		name = "HDMI Capture (Media Foundation)";
	}

	virtual ~WinMFCapture() {
		Close();
	}

	String GetName() const override { return name; }
	Size GetSize() const override { return size; }

	bool Open() override {
		if (opened) return true;

		// 1) Enumeroi videokaappauslaitteet
		ComPtr<IMFAttributes> attrs;
		if (FAILED(MFCreateAttributes(&attrs, 1))) return false;
		if (FAILED(attrs->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) return false;

		IMFActivate** devices = nullptr;
		UINT32 count = 0;
		if (FAILED(MFEnumDeviceSources(attrs.Get(), &devices, &count)) || count == 0) return false;

		// Aktivoi ensimmäinen kamera
		ComPtr<IMFMediaSource> source;
		HRESULT hr = devices[0]->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void**>(source.GetAddressOf()));
		
		for (UINT32 i = 0; i < count; ++i) devices[i]->Release();
		CoTaskMemFree(devices);

		if (FAILED(hr)) return false;

		// 3) Luo SourceReader
		if (FAILED(MFCreateSourceReaderFromMediaSource(source.Get(), nullptr, &reader))) return false;

		// 4) Aseta haluttu ulostulomuoto
		ComPtr<IMFMediaType> type;
		if (FAILED(MFCreateMediaType(&type))) return false;
		if (FAILED(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video))) return false;
		if (FAILED(type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32))) return false;
		
		if (FAILED(reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, type.Get()))) return false;

		opened = true;
		return true;
	}

	void Close() override {
		reader.Reset();
		opened = false;
	}

	Image GrabFrame(bool force_fetch = false) override {
		if (!opened || !reader) return Null;

		DWORD streamIndex = 0, flags = 0;
		LONGLONG timestamp = 0;
		ComPtr<IMFSample> sample;

		HRESULT hr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &timestamp, &sample);
		if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM) || !sample) return Null;

		ComPtr<IMFMediaBuffer> buffer;
		if (FAILED(sample->ConvertToContiguousBuffer(&buffer))) return Null;

		BYTE* data = nullptr;
		DWORD maxLen = 0, curLen = 0;
		if (FAILED(buffer->Lock(&data, &maxLen, &curLen))) return Null;

		ImageBuffer ib(size);
		int stride = size.cx * 4;
		if (curLen >= (DWORD)(size.cy * stride)) {
			for (int y = 0; y < size.cy; y++) {
				const byte* src_row = data + (size.cy - 1 - y) * stride;
				RGBA* dst_row = ib[y];
				for(int x = 0; x < size.cx; x++) {
					dst_row[x].r = src_row[x * 4 + 2];
					dst_row[x].g = src_row[x * 4 + 1];
					dst_row[x].b = src_row[x * 4 + 0];
					dst_row[x].a = 255;
				}
			}
		}

		buffer->Unlock();
		return ib;
	}
};
#endif

// --- Screenshot Simulation Capture ---

class SimCapture : public SimCaptureSource {
	Vector<String> files;
	int index = 0;
	Size size;
	bool opened = false;

public:
	SimCapture() {
		size = Size(1920, 1080);
	}

	String GetName() const override { return "Simulation (Screenshots)"; }
	Size GetSize() const override { return size; }

	bool LoadFolder(const String& path) override {
		files.Clear();
		for(FindFile ff(AppendFileName(path, "*.*")); ff; ff.Next()) {
			if (ff.IsFile()) {
				String ext = ToLower(GetFileExt(ff.GetName()));
				if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
					files.Add(ff.GetPath());
				}
			}
		}
		Sort(files);
		return files.GetCount() > 0;
	}

	bool Open() override {
		if (files.IsEmpty()) return false;
		opened = true;
		return true;
	}

	void Close() override {
		opened = false;
	}

	Image GrabFrame(bool force_fetch = false) override {
		if (!opened || files.IsEmpty()) return Null;
		Image img = StreamRaster::LoadFileAny(files[index]);
		if (files.GetCount() > 0)
			index = (index + 1) % files.GetCount();
		
		if (img.GetSize() != size && !img.IsEmpty()) {
			img = Rescale(img, size);
		}
		return img;
	}
};

// --- Remote TCP Capture ---

class RemoteCapture : public RemoteCaptureSource {
	TcpSocket socket;
	String host;
	int port;
	bool opened = false;
	Size size = Size(1920, 1080);
	uint32 last_id = 0;

public:
	bool Connect(const String& h, int p) override {
		host = h; port = p;
		return Open();
	}

	bool Open() override {
		socket.Timeout(2000);
		if (socket.Connect(host, port)) {
			opened = true;
			last_id = 0;
			return true;
		}
		return false;
	}

	void Close() override {
		socket.Close();
		opened = false;
	}

	Image GrabFrame(bool force_fetch = false) override {
		if (!opened) return Null;
		
		// Pull: send last seen ID
		uint32 req_id = force_fetch ? 0 : last_id;
		socket.Put(&req_id, 4);
		
		uint32 resp_id = 0;
		uint32 sz = 0;
		if (socket.GetAll(&resp_id, 4) && socket.GetAll(&sz, 4)) {
			if (sz > 0) {
				String jpeg = socket.GetAll(sz);
				if (jpeg.GetCount() == (int)sz) {
					last_id = resp_id;
					Image img = JPGRaster().LoadString(jpeg);
					if (img.IsEmpty()) {
						Cout() << "DEBUG: JPEG decoding failed! sz=" << sz << " id=" << resp_id << "\n";
					}
					return img;
				} else {
					Cout() << "DEBUG: GetAll(sz) failed! got=" << jpeg.GetCount() << " expected=" << sz << "\n";
				}
			} else {
				// No new frames (resp_id == last_id)
				return Null;
			}
		} else {
			if (socket.IsError()) Cout() << "DEBUG: socket error: " << socket.GetErrorDesc() << "\n";
			if (socket.IsEof()) Cout() << "DEBUG: socket EOF\n";
		}
		
		if (socket.IsError() || socket.IsEof()) {
			Close();
		}
		
		return Null;
	}

	Size GetSize() const override { return size; }
	String GetName() const override { return "Remote TCP (" + host + ":" + AsString(port) + ")"; }
};

CaptureSource* CreateWinMFCapture() {
#ifdef PLATFORM_WIN32
	return new WinMFCapture();
#else
	return nullptr;
#endif
}

CaptureSource* CreateV4l2Capture(const String& device, const String& format_policy) {
#ifdef PLATFORM_WIN32
	return nullptr;
#else
	// Implementation is in V4l2Capture.cpp
	extern CaptureSource* CreateV4l2CaptureImpl(const String& device, const String& format_policy);
	return CreateV4l2CaptureImpl(device, format_policy);
#endif
}
SimCaptureSource* CreateSimCapture() { return new SimCapture(); }
RemoteCaptureSource* CreateRemoteCapture() { return new RemoteCapture(); }

void InitCapture() {
#ifdef PLATFORM_WIN32
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	MFStartup(MF_VERSION);
#endif
}

void ExitCapture() {
#ifdef PLATFORM_WIN32
	MFShutdown();
	CoUninitialize();
#endif
}

}
