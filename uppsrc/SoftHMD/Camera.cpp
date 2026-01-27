#include "SoftHMD.h"
#include "Camera.h"
#include <Core/Core.h>
#include <libusb-1.0/libusb.h>

NAMESPACE_HMD_BEGIN

#define WMR_VID 0x045e
#define WMR_PID 0x0659
#define WMR_INTERFACE_CONTROL 0 // Usually 2, but let's see
#define WMR_VIDEO_INTERFACE 3
#define WMR_VIDEO_INTERFACE2 4
#define WMR_VIDEO_ENDPOINT 0x85
#define WMR_COMMAND_ENDPOINT 0x05
#define WMR_COMMAND_ENDPOINT2 0x06
#define WMR_BULK_SIZE 616538
#define WMR_PACKET_SIZE 0x6000
#define WMR_HEADER_SIZE 0x20

#define HOLOLENS_CAMERA2_MAGIC	0x2b6f6c44

struct hololens_camera2_command {
	uint32 magic;
	uint32 len;
	uint32 command;
} __attribute__((packed));

struct hololens_camera2_unknown_command {
	uint32 magic;
	uint32 len;
	uint16 command;
	uint16 camera; /* 0 for left, 1 for right */
	uint16 unknown_6000;
	uint16 gain; /* observed 82 to 255 */
	uint16 second_camera; /* same as camera */
} __attribute__((packed));

#ifdef CPU_LE
#define CpuToLE32(x) (x)
#define CpuToLE16(x) (x)
#else
#define CpuToLE32(x) (SwapEndian(x))
#define CpuToLE16(x) (SwapEndian(x))
#endif

static int wmr_camera_send(libusb_device_handle* usb_handle, void *buf, size_t len)
{
	int transferred = 0;
	// Try endpoint 0x05 first
	int r = libusb_bulk_transfer(usb_handle, WMR_COMMAND_ENDPOINT, (byte*)buf, (int)len, &transferred, 1000);
	if(r != 0) {
		// Try endpoint 0x06
		r = libusb_bulk_transfer(usb_handle, WMR_COMMAND_ENDPOINT2, (byte*)buf, (int)len, &transferred, 1000);
	}
	if(r != 0) {
		Cout() << "Camera command error: " << libusb_error_name(r) << " (" << r << "), len=" << (int)len << "\n";
	}
	return r;
}

static void wmr_camera_set_active(libusb_device_handle* usb_handle, bool active)
{
	struct hololens_camera2_command command;
	command.magic = CpuToLE32(HOLOLENS_CAMERA2_MAGIC);
	command.len = CpuToLE32(sizeof(command));
	command.command = CpuToLE32(active ? 0x81 : 0x82);

	wmr_camera_send(usb_handle, &command, sizeof(command));
}

static void wmr_camera_set_gain(libusb_device_handle* usb_handle, uint8 camera, uint8 gain)
{
	struct hololens_camera2_unknown_command command;
	command.magic = CpuToLE32(HOLOLENS_CAMERA2_MAGIC);
	command.len = CpuToLE32(sizeof(command));
	command.command = CpuToLE16(0x80);
	command.camera = CpuToLE16(camera);
	command.unknown_6000 = CpuToLE16(6000);
	command.gain = CpuToLE16(gain);
	command.second_camera = CpuToLE16(camera);

	wmr_camera_send(usb_handle, &command, sizeof(command));
}

HMD_APIENTRYDLL Camera::Camera()
{
	opened = false;
	quit = false;
	usb_ctx = NULL;
	usb_handle = NULL;
	frame_buffer.Alloc(2000000);
	raw_buffer.Alloc(2000000);
	stats.frame_count = 0;
}

HMD_APIENTRYDLL Camera::~Camera()
{
	Close();
}

Camera::Stats HMD_APIENTRYDLL Camera::GetStats()
{
	Upp::Mutex::Lock __(mutex);
	return stats;
}

Image HMD_APIENTRYDLL Camera::GetImage()
{
	Upp::Mutex::Lock __(mutex);
	int w = 1280;
	int h = 480;
	ImageBuffer ib(w, h * 2);
	if(bright_img) {
		for(int y = 0; y < h; y++)
			memcpy(ib[y], bright_img[y], w * sizeof(RGBA));
	}
	else {
		for(int y = 0; y < h; y++)
			memset(ib[y], 0, w * sizeof(RGBA));
	}
	
	if(dark_img) {
		for(int y = 0; y < h; y++)
			memcpy(ib[y + h], dark_img[y], w * sizeof(RGBA));
	}
	else {
		for(int y = 0; y < h; y++)
			memset(ib[y + h], 0, w * sizeof(RGBA));
	}
	
	return ib;
}

bool HMD_APIENTRYDLL Camera::Open()
{
	if(opened) return true;
	
	if(libusb_init(&usb_ctx) != 0) return false;
	
	usb_handle = libusb_open_device_with_vid_pid(usb_ctx, WMR_VID, WMR_PID);
	if(!usb_handle) {
		libusb_exit(usb_ctx);
		usb_ctx = NULL;
		return false;
	}
	
	libusb_reset_device(usb_handle);
	libusb_set_auto_detach_kernel_driver(usb_handle, 1);

	libusb_device *dev = libusb_get_device(usb_handle);
	struct libusb_config_descriptor *config_desc;
	if (libusb_get_active_config_descriptor(dev, &config_desc) == 0) {
		Cout() << "USB Config: interfaces=" << (int)config_desc->bNumInterfaces << "\n";
		for (int i = 0; i < config_desc->bNumInterfaces; i++) {
			const struct libusb_interface *intf = &config_desc->interface[i];
			Cout() << "  Interface " << i << ": alt_settings=" << intf->num_altsetting << "\n";
			for (int j = 0; j < intf->num_altsetting; j++) {
				const struct libusb_interface_descriptor *id = &intf->altsetting[j];
				Cout() << "    Alt " << j << ": num=" << (int)id->bInterfaceNumber << ", class=" << (int)id->bInterfaceClass << ", endpoints=" << (int)id->bNumEndpoints << "\n";
				for (int k = 0; k < id->bNumEndpoints; k++) {
					const struct libusb_endpoint_descriptor *ed = &id->endpoint[k];
					Cout() << "      Endpoint " << k << ": addr=" << FormatIntHex((int)ed->bEndpointAddress) << ", attr=" << (int)ed->bmAttributes << "\n";
				}
			}
		}
		libusb_free_config_descriptor(config_desc);
	}
	
	for(int i = 2; i <= 4; i++) {
		int r = libusb_claim_interface(usb_handle, i);
		if(r != 0) {
			Cout() << "Failed to claim interface " << i << ": " << libusb_error_name(r) << "\n";
		}
		libusb_set_interface_alt_setting(usb_handle, i, 0);
	}
	
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT);
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT2);
	libusb_clear_halt(usb_handle, WMR_VIDEO_ENDPOINT);
	
	wmr_camera_set_active(usb_handle, false);
	Upp::Sleep(200);
	wmr_camera_set_active(usb_handle, true);
	Upp::Sleep(200);
	wmr_camera_set_gain(usb_handle, 0, 0x80); // left
	Upp::Sleep(100);
	wmr_camera_set_gain(usb_handle, 1, 0x80); // right
	Upp::Sleep(100);
	
	// Drain any old data
	int transferred = 0;
	libusb_bulk_transfer(usb_handle, WMR_VIDEO_ENDPOINT, frame_buffer, 2000000, &transferred, 10);

	quit = false;
	thread.Start(THISBACK(Process));
	opened = true;
	
	Cout() << "HMD Camera opened successfully\n";
	
	return true;
}

void HMD_APIENTRYDLL Camera::Close()
{
	if(!opened) return;
	
	quit = true;
	thread.Wait();
	
	if(usb_handle) {
		wmr_camera_set_active(usb_handle, false);
		for(int i = 2; i <= 4; i++) {
			libusb_release_interface(usb_handle, i);
		}
		libusb_close(usb_handle);
	}
	if(usb_ctx) {
		libusb_exit(usb_ctx);
	}
	
	usb_handle = NULL;
	usb_ctx = NULL;
	opened = false;
}

void Camera::Process()
{
	while(!quit) {
		int transferred = 0;
		int r = libusb_bulk_transfer(usb_handle, WMR_VIDEO_ENDPOINT, frame_buffer, WMR_BULK_SIZE + 4000, &transferred, 1000);
		{
			Upp::Mutex::Lock __(mutex);
			stats.last_transferred = transferred;
			stats.last_error = r;
			stats.last_r = r;
			if(transferred > 0) {
				if(transferred < stats.min_transferred) stats.min_transferred = transferred;
				if(transferred > stats.max_transferred) stats.max_transferred = transferred;
			}
		}
		if(r == 0 && transferred > 0) {
			HandleFrame(frame_buffer, transferred);
		}
		else if(r == LIBUSB_ERROR_TIMEOUT) {
			// Ignore timeout, just retry
		}
		else if(r == LIBUSB_ERROR_OVERFLOW) {
			Cout() << "USB Overflow, re-activating camera\n";
			wmr_camera_set_active(usb_handle, true);
		}
		else {
			// Error, wait a bit and retry
			Upp::Sleep(10);
		}
	}
}

void Camera::HandleFrame(const byte* buffer, int size)
{
	/* Strip out packet headers */
	int j = 0;
	for (int i = 0; i < size; i += WMR_PACKET_SIZE) {
		int payload_offset = i + WMR_HEADER_SIZE;
		if (payload_offset >= size)
			break;
		
		int n = WMR_PACKET_SIZE - WMR_HEADER_SIZE;
		if (i + WMR_PACKET_SIZE > size)
			n = size - payload_offset;
		
		if(j + n > 2000000) break; // Safety
		
		memcpy(raw_buffer + j, buffer + payload_offset, n);
		j += n;
	}
	
	// Need at least metadata line (1280) + one pixel line (1280)
	if(j < 1280 * 2) return;

	// Exposure is at offset 6 of the metadata line (Big Endian)
	uint16 exposure = (raw_buffer[6] << 8) | raw_buffer[7];
	bool is_bright = (exposure != 0);
	
	// Image data starts after metadata line (1280 bytes)
	// Resolution is 1280x480 (or 2x 640x480)
	int width = 1280;
	int height = 480;
	int total_pixels = width * height;
	
	if(j < 1280 + total_pixels) return;

	ImageBuffer ib(width, height);
	const byte* src = raw_buffer + 1280;
	
	uint64 sum = 0;
	byte min_p = 255;
	byte max_p = 0;
	
	RGBA* dst = ib;
	for(int p = 0; p < total_pixels; p++) {
		byte v = src[p];
		dst[p] = GrayColor(v);
		sum += v;
		if(v < min_p) min_p = v;
		if(v > max_p) max_p = v;
	}
	
	Upp::Mutex::Lock __(mutex);
	if(is_bright) {
		bright_img = ib;
	}
	else {
		dark_img = ib;
	}
	
	stats.frame_count++;
	if(is_bright) stats.bright_frames++;
	else stats.dark_frames++;
	stats.last_exposure = exposure;
	stats.avg_brightness = (double)sum / total_pixels;
	stats.min_pixel = min_p;
	stats.max_pixel = max_p;
}

NAMESPACE_HMD_END
