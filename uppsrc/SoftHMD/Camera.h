#ifndef _SoftHMD_Camera_h_
#define _SoftHMD_Camera_h_

extern "C" {
#include <libusb-1.0/libusb.h>
}

NAMESPACE_HMD_BEGIN


class Camera {
public:
	HMD_APIENTRYDLL Camera();
	HMD_APIENTRYDLL ~Camera();

	bool HMD_APIENTRYDLL Open();
	void HMD_APIENTRYDLL Close();
	
	bool HMD_APIENTRYDLL IsOpen() const { return opened; }
	
	Image HMD_APIENTRYDLL GetImage();
	
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

	Stats HMD_APIENTRYDLL GetStats();
	
	typedef Camera CLASSNAME;

private:
	void Process();
	void HandleFrame(const byte* buffer, int size);

	bool opened;
	bool quit;
	
	Upp::Thread thread;
	Upp::Mutex mutex;
	Image bright_img, dark_img;
	Stats stats;
	
	libusb_context* usb_ctx;
	libusb_device_handle* usb_handle;
	
	Buffer<byte> frame_buffer;
	Buffer<byte> raw_buffer;
};


NAMESPACE_HMD_END

#endif
