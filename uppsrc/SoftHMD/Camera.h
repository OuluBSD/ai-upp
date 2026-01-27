#ifndef _SoftHMD_Camera_h_
#define _SoftHMD_Camera_h_

extern "C" {
#include <libusb-1.0/libusb.h>
}
#include <vector>

NAMESPACE_HMD_BEGIN

class HMD_APIENTRYDLL Camera {
public:
	struct CameraFrame : public Moveable<CameraFrame> {
		Image img;
		bool is_bright;
		int exposure;
	};

	struct Stats {
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
		
		Stats() { 
			memset(this, 0, sizeof(*this)); 
			min_transferred = 1000000000;
		}
	};

	Camera();
	~Camera();

	bool Open();
	void Close();
	
	bool IsOpen() const { return opened; }
	
	void PopFrames(Vector<CameraFrame>& out);
	Stats GetStats();
	
	typedef Camera CLASSNAME;

	static void LIBUSB_CALL TransferCallback(struct libusb_transfer* transfer);

private:
	void Process();
	void HandleFrame(const byte* buffer, int size);

	bool opened;
	bool quit;
	
	Upp::Thread thread;
	Upp::Mutex mutex;
	Vector<CameraFrame> queue;
	Stats stats;
	
	libusb_context* usb_ctx;
	libusb_device_handle* usb_handle;
	
	static const int ASYNC_BUFFERS = 4;
	struct Transfer {
		struct libusb_transfer* libusb_xfer;
		byte* buffer;
		Camera* camera;
	} transfers[ASYNC_BUFFERS];
	
	std::vector<byte> raw_buffer;
};


NAMESPACE_HMD_END

#endif
