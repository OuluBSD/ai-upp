#ifndef _SoftHMD_Camera_h_
#define _SoftHMD_Camera_h_

extern "C" {
#include <libusb-1.0/libusb.h>
}
#include <vector>

NAMESPACE_HMD_BEGIN

struct CameraFrame : public Moveable<CameraFrame> {
	Image img;
	bool is_bright;
	int exposure;
};

struct CameraStats {
	int frame_count;
	int bright_frames;
	int dark_frames;
	int last_exposure;
	int last_transferred;
	int last_error;
	int last_r;
	int min_transferred, max_transferred;
	double avg_brightness;
	byte min_pixel, max_pixel;
	
	int mutex_fails;
	int usb_errors;
	int timeout_errors;
	int overflow_errors;
	int other_errors;
	
	int bright_balance; // bright_frames - dark_frames
	int handle_usecs;
	
	CameraStats() { 
		memset(this, 0, sizeof(*this)); 
		min_transferred = 1000000000;
	}
};

class Camera {
public:
	HMD_APIENTRYDLL Camera();
	HMD_APIENTRYDLL ~Camera();

	bool HMD_APIENTRYDLL Open();
	void HMD_APIENTRYDLL Close();
	
	bool HMD_APIENTRYDLL IsOpen() const { return opened; }
	
	void HMD_APIENTRYDLL PopFrames(Vector<CameraFrame>& out);
	CameraStats HMD_APIENTRYDLL GetStats();
	void HMD_APIENTRYDLL SetVerbose(bool v) { verbose = v; }
	
	typedef Camera CLASSNAME;

	static void LIBUSB_CALL TransferCallback(struct libusb_transfer* transfer);

private:
	void Process();
	void HandleFrame(const byte* buffer, int size);

	bool opened;
	bool quit;
	bool verbose;
	
	Upp::Thread thread;
	Upp::Mutex mutex;
	Vector<CameraFrame> queue;
	CameraStats stats;
	
	libusb_context* usb_ctx;
	libusb_device_handle* usb_handle;
	
	static const int ASYNC_BUFFERS = 4;
	struct Transfer {
		struct libusb_transfer* libusb_xfer;
		byte* buffer;
		Camera* camera;
	} transfers[ASYNC_BUFFERS];
	
	std::vector<byte> raw_buffer;
	int raw_buffer_ptr;
	std::vector<byte> stripped_buffer;
	int64 last_halt_clear_usecs;
};


NAMESPACE_HMD_END

#endif
