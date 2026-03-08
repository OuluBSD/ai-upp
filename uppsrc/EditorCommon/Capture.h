#ifndef _ScreenGame_Capture_h_
#define _ScreenGame_Capture_h_

#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>
#include <EditorCommon/GpuPreprocess.h>

namespace Upp {

class CaptureSource {
public:
	virtual ~CaptureSource() {}
	virtual bool Open() = 0;
	virtual void Close() = 0;
	virtual Image GrabFrame(bool force_fetch = false) = 0;
	virtual bool GrabGpuFrame(GpuFrame& out) { return false; } // Phase 12: Zero-copy support
	virtual Size GetSize() const = 0;
	virtual String GetName() const = 0;
};

// Windows Media Foundation Capture
CaptureSource* CreateWinMFCapture();

// Linux V4L2 Capture
CaptureSource* CreateV4l2Capture(const String& device = "/dev/video0", const String& format_policy = "auto");

// Lifecycle
void InitCapture();
void ExitCapture();

// Screenshot Simulation Capture
class SimCaptureSource : public CaptureSource {
public:
	virtual bool LoadFolder(const String& path) = 0;
};

SimCaptureSource* CreateSimCapture();

// Remote TCP Capture
class RemoteCaptureSource : public CaptureSource {
public:
	virtual bool Connect(const String& host, int port) = 0;
};

RemoteCaptureSource* CreateRemoteCapture();

}

#endif
