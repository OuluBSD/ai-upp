#include "SoftHMD.h"
#include "Camera.h"
#include <Core/Core.h>
#include <libusb-1.0/libusb.h>

NAMESPACE_HMD_BEGIN

#define WMR_VID 0x045e
#define WMR_PID 0x0659
#define WMR_INTERFACE_CONTROL 0
#define WMR_VIDEO_INTERFACE 3
#define WMR_VIDEO_INTERFACE2 4
#define WMR_VIDEO_ENDPOINT 0x85
#define WMR_COMMAND_ENDPOINT 0x05
#define WMR_COMMAND_ENDPOINT2 0x06
#define WMR_BULK_SIZE 616538
#define WMR_PACKET_SIZE 0x6000
#define WMR_HEADER_SIZE 0x20

#define HOLOLENS_CAMERA2_MAGIC	0x2b6f6c44

#ifdef CPU_LE
#define CpuToLE32(x) (x)
#define CpuToLE16(x) (x)
#else
#define CpuToLE32(x) (SwapEndian(x))
#define CpuToLE16(x) (SwapEndian(x))
#endif

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

static int wmr_camera_send(libusb_device_handle* usb_handle, void *buf, size_t len)
{
	int transferred = 0;
	int r = libusb_bulk_transfer(usb_handle, WMR_COMMAND_ENDPOINT, (byte*)buf, (int)len, &transferred, 1000);
	if(r != 0) {
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

Camera::Camera()
{
	opened = false;
	quit = false;
	usb_ctx = NULL;
	usb_handle = NULL;
	raw_buffer.assign(2000000, 0);
	stats.frame_count = 0;
	for(int i = 0; i < ASYNC_BUFFERS; i++) {
		transfers[i].libusb_xfer = NULL;
		transfers[i].buffer = NULL;
	}
}

Camera::~Camera()
{
	Close();
}

void LIBUSB_CALL Camera::TransferCallback(struct libusb_transfer* xfer)
{
	Transfer* t = (Transfer*)xfer->user_data;
	Camera* cam = t->camera;
	
	if(xfer->status == LIBUSB_TRANSFER_COMPLETED) {
		{
			Upp::Mutex::Lock __(cam->mutex);
			cam->stats.last_transferred = xfer->actual_length;
			cam->stats.last_error = 0;
			cam->stats.last_r = 0;
			if(xfer->actual_length > 0) {
				if(xfer->actual_length < cam->stats.min_transferred) cam->stats.min_transferred = xfer->actual_length;
				if(xfer->actual_length > cam->stats.max_transferred) cam->stats.max_transferred = xfer->actual_length;
			}
		}
		
		if(xfer->actual_length > 0) {
			cam->HandleFrame(xfer->buffer, xfer->actual_length);
		}
	}
	else {
		Upp::Mutex::Lock __(cam->mutex);
		cam->stats.last_error = xfer->status;
		cam->stats.last_r = xfer->status;
	}
	
	if(!cam->quit) {
		if(libusb_submit_transfer(xfer) != 0) {
		}
	}
}

void Camera::PopFrames(Vector<CameraFrame>& out)
{
	Upp::Mutex::Lock __(mutex);
	out = pick(queue);
}

Camera::Stats Camera::GetStats()
{
	Upp::Mutex::Lock __(mutex);
	return stats;
}

bool HMD_APIENTRYDLL Camera::Open()
{
	if(opened) return true;
	
	if(libusb_init(NULL) != 0) return false;
	
	usb_handle = libusb_open_device_with_vid_pid(NULL, WMR_VID, WMR_PID);
	if(!usb_handle) {
		libusb_exit(NULL);
		return false;
	}
	
	libusb_set_auto_detach_kernel_driver(usb_handle, 1);

	libusb_device *dev = libusb_get_device(usb_handle);
	struct libusb_config_descriptor *config_desc;
	if (libusb_get_active_config_descriptor(dev, &config_desc) == 0) {
		Cout() << "USB Config: interfaces=" << (int)config_desc->bNumInterfaces << "\n";
		for (int i = 0; i < config_desc->bNumInterfaces; i++) {
			const struct libusb_interface *intf = &config_desc->interface[i];
			for (int j = 0; j < intf->num_altsetting; j++) {
				const struct libusb_interface_descriptor *id = &intf->altsetting[j];
				Cout() << "    Alt " << j << ": num=" << (int)id->bInterfaceNumber << ", class=" << (int)id->bInterfaceClass << ", endpoints=" << (int)id->bNumEndpoints << "\n";
			}
		}
		libusb_free_config_descriptor(config_desc);
	}
	
	for(int i = 2; i <= 4; i++) {
		if(libusb_claim_interface(usb_handle, i) != 0) {
			Cout() << "Failed to claim interface " << i << "\n";
		}
		libusb_set_interface_alt_setting(usb_handle, i, 0);
	}
	
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT);
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT2);
	libusb_clear_halt(usb_handle, WMR_VIDEO_ENDPOINT);
	libusb_clear_halt(usb_handle, 0x84);
	
	wmr_camera_set_active(usb_handle, false);
	Upp::Sleep(200);
	wmr_camera_set_active(usb_handle, true);
	Upp::Sleep(200);
	wmr_camera_set_gain(usb_handle, 0, 0x80); // left
	wmr_camera_set_gain(usb_handle, 1, 0x80); // right
	Upp::Sleep(100);
	
	raw_buffer.assign(2000000, 0);
	std::vector<byte> frame_buffer_sync(WMR_BULK_SIZE + 4096);

	quit = false;
	thread.Start([this, frame_buffer_sync]() mutable {
		while(!quit) {
			int transferred = 0;
			int r = libusb_bulk_transfer(usb_handle, WMR_VIDEO_ENDPOINT, frame_buffer_sync.data(), (int)frame_buffer_sync.size(), &transferred, 1000);
			if(r == 0 && transferred > 0) {
				HandleFrame(frame_buffer_sync.data(), transferred);
			}
			else if(r != LIBUSB_ERROR_TIMEOUT) {
				Upp::Sleep(10);
			}
		}
	});
	opened = true;
	
	Cout() << "HMD Camera opened successfully (Sync mode)\n";
	
	return true;
}

void Camera::Close()
{
	if(!opened) return;
	
	quit = true;
	thread.Wait();
	
	if(usb_handle) {
		wmr_camera_set_active(usb_handle, false);
		for(int i = 3; i <= 4; i++) {
			libusb_release_interface(usb_handle, i);
		}
		libusb_close(usb_handle);
	}
	
	libusb_exit(NULL);
	
	usb_handle = NULL;
	opened = false;
}

void Camera::Process()
{
	std::vector<byte> hid_buffer(512);
	std::vector<byte> video_buffer(WMR_BULK_SIZE + 4096);
	while(!quit) {
		// Try reading HID sensors
		int transferred = 0;
		int r = libusb_interrupt_transfer(usb_handle, 0x84, hid_buffer.data(), (int)hid_buffer.size(), &transferred, 10);
		if (r == 0 && transferred > 0) {
			Cout() << "HID: size=" << transferred << ", byte0=" << (int)hid_buffer[0] << "\n";
		} else if (r != 0 && r != LIBUSB_ERROR_TIMEOUT) {
			Cout() << "HID error: " << libusb_error_name(r) << " (" << r << ")\n";
		}

		// Try reading Video
		transferred = 0;
		r = libusb_bulk_transfer(usb_handle, WMR_VIDEO_ENDPOINT, video_buffer.data(), (int)video_buffer.size(), &transferred, 10);
		if(r == 0 && transferred > 0) {
			HandleFrame(video_buffer.data(), transferred);
		}
	}
}

void Camera::HandleFrame(const byte* buffer, int size)
{
	int j = 0;
	for (int i = 0; i < size; i += WMR_PACKET_SIZE) {
		int payload_offset = i + WMR_HEADER_SIZE;
		if (payload_offset >= size)
			break;
		int n = WMR_PACKET_SIZE - WMR_HEADER_SIZE;
		if (i + WMR_PACKET_SIZE > size)
			n = size - payload_offset;
		if(j + n > (int)raw_buffer.size()) break;
		memcpy(raw_buffer.data() + j, buffer + payload_offset, n);
		j += n;
	}
	
	if(j < 615680) return;

	uint16 exposure = (raw_buffer[6] << 8) | raw_buffer[7];
	bool is_bright = (exposure != 0);
	
	int width = 1280;
	int height = 481;
	int total_pixels = width * height;
	
	if(j < 26 + total_pixels) return;

	ImageBuffer ib(width, height);
	const byte* src = raw_buffer.data() + 26;
	
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
	
	{
		Upp::Mutex::Lock __(mutex);
		if(queue.GetCount() > 30) queue.Remove(0);
		CameraFrame& f = queue.Add();
		f.img = ib;
		f.is_bright = is_bright;
		f.exposure = exposure;
		
		stats.frame_count++;
		if(is_bright) stats.bright_frames++;
		else stats.dark_frames++;
		stats.last_exposure = exposure;
		stats.avg_brightness = (double)sum / total_pixels;
		stats.min_pixel = min_p;
		stats.max_pixel = max_p;
	}
}

NAMESPACE_HMD_END
