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
	raw_buffer.assign(4000000, 0); // 4MB buffer
	raw_buffer_ptr = 0;
	stripped_buffer.assign(1000000, 0); // 1MB buffer
	last_halt_clear_usecs = 0;
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
		if(xfer->actual_length > 0) {
			if(cam->mutex.TryEnter()) {
				cam->stats.last_transferred = xfer->actual_length;
				if(xfer->actual_length < cam->stats.min_transferred)
					cam->stats.min_transferred = xfer->actual_length;
				if(xfer->actual_length > cam->stats.max_transferred)
					cam->stats.max_transferred = xfer->actual_length;
				cam->stats.last_error = 0;
				cam->stats.last_r = 0;
				cam->mutex.Leave();
			} else {
				cam->stats.mutex_fails++;
			}
			cam->HandleFrame(xfer->buffer, xfer->actual_length);
		} else {
			if(cam->verbose) Cout() << "USB: Zero-byte transfer completed\n";
		}
	}
	else {
		Cout() << "USB Error: Transfer status " << (int)xfer->status << "\n";
		if(cam->mutex.TryEnter()) {
			cam->stats.last_error = xfer->status;
			cam->stats.last_r = xfer->status;
			cam->stats.usb_errors++;
			if (xfer->status == LIBUSB_TRANSFER_TIMED_OUT) cam->stats.timeout_errors++;
			else if (xfer->status == LIBUSB_TRANSFER_OVERFLOW) cam->stats.overflow_errors++;
			else cam->stats.other_errors++;
			cam->mutex.Leave();
		} else {
			cam->stats.mutex_fails++;
			Cout() << "USB Error: Mutex locked during error reporting\n";
		}
	}
	
	if(!cam->quit) {
		if(xfer->status == LIBUSB_TRANSFER_ERROR || xfer->status == LIBUSB_TRANSFER_STALL) {
			bool should_clear = true;
			if(cam->mutex.TryEnter()) {
				const int64 now = usecs();
				if(cam->last_halt_clear_usecs != 0 && now - cam->last_halt_clear_usecs < 500000)
					should_clear = false;
				if(should_clear)
					cam->last_halt_clear_usecs = now;
				cam->mutex.Leave();
			}
			if(should_clear) {
				int ch = libusb_clear_halt(cam->usb_handle, WMR_VIDEO_ENDPOINT);
				if(ch != 0)
					Cout() << "USB Error: Failed to clear halt: " << libusb_error_name(ch) << " (" << ch << ")\n";
			}
		}
		int r = libusb_submit_transfer(xfer);
		if (r != 0) {
			Cout() << "USB Error: Failed to resubmit transfer: " << libusb_error_name(r) << " (" << r << ")\n";
			if(cam->mutex.TryEnter()) {
				cam->stats.usb_errors++;
				cam->stats.last_r = r;
				cam->mutex.Leave();
			}
		}
	}
}

void HMD_APIENTRYDLL Camera::PopFrames(Vector<CameraFrame>& out)
{
	Upp::Mutex::Lock __(mutex);
	out = pick(queue);
}

CameraStats HMD_APIENTRYDLL Camera::GetStats()
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
	
	// libusb_set_auto_detach_kernel_driver(usb_handle, 1);

	for(int i = 3; i <= 4; i++) {
		libusb_claim_interface(usb_handle, i);
		libusb_set_interface_alt_setting(usb_handle, i, 0);
	}
	
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT);
	libusb_clear_halt(usb_handle, WMR_COMMAND_ENDPOINT2);
	libusb_clear_halt(usb_handle, WMR_VIDEO_ENDPOINT);
	
	wmr_camera_set_active(usb_handle, false);
	Upp::Sleep(200);
	wmr_camera_set_active(usb_handle, true);
	Upp::Sleep(200);
	wmr_camera_set_gain(usb_handle, 0, 0x80);
	wmr_camera_set_gain(usb_handle, 1, 0x80);
	Upp::Sleep(100);
	
	for(int i = 0; i < ASYNC_BUFFERS; i++) {
		transfers[i].camera = this;
		transfers[i].buffer = (byte*)malloc(WMR_BULK_SIZE + 4096);
		transfers[i].libusb_xfer = libusb_alloc_transfer(0);
		libusb_fill_bulk_transfer(transfers[i].libusb_xfer, usb_handle, WMR_VIDEO_ENDPOINT,
		                          transfers[i].buffer, WMR_BULK_SIZE + 4096,
		                          TransferCallback, &transfers[i], 0);
		libusb_submit_transfer(transfers[i].libusb_xfer);
	}

	quit = false;
	thread.Start(THISBACK(Process));
	opened = true;
	
	return true;
}

void HMD_APIENTRYDLL Camera::Close()
{
	if(!opened) return;
	
	quit = true;
	for(int i = 0; i < ASYNC_BUFFERS; i++) {
		if(transfers[i].libusb_xfer)
			libusb_cancel_transfer(transfers[i].libusb_xfer);
	}
	
	thread.Wait();
	
	if(usb_handle) {
		wmr_camera_set_active(usb_handle, false);
		for(int i = 3; i <= 4; i++) {
			libusb_release_interface(usb_handle, i);
		}
		libusb_close(usb_handle);
	}
	
	libusb_exit(NULL);
	
	for(int i = 0; i < ASYNC_BUFFERS; i++) {
		if(transfers[i].libusb_xfer)
			libusb_free_transfer(transfers[i].libusb_xfer);
		if(transfers[i].buffer)
			free(transfers[i].buffer);
		transfers[i].libusb_xfer = NULL;
		transfers[i].buffer = NULL;
	}
	
	usb_handle = NULL;
	opened = false;
}

void Camera::Process()
{
	struct timeval tv = { 0, 10000 };
	while(!quit) {
		libusb_handle_events_timeout_completed(NULL, &tv, NULL);
	}
}

void Camera::HandleFrame(const byte* buffer, int size)
{
	int64 start_usecs = usecs();
	if(verbose) Cout() << "HandleFrame: size=" << size << ", raw_ptr=" << raw_buffer_ptr << "\n";

	if(raw_buffer_ptr + size > (int)raw_buffer.size()) {
		if(verbose) Cout() << "Buffer overflow, resetting raw_buffer_ptr\n";
		raw_buffer_ptr = 0;
	}
	memcpy(raw_buffer.data() + raw_buffer_ptr, buffer, size);
	raw_buffer_ptr += size;
	
	const int RAW_FRAME_SIZE = 616538;
	const int STRIPPED_FRAME_SIZE = 615706;
	
	while(raw_buffer_ptr >= RAW_FRAME_SIZE) {
		uint16 exposure = (raw_buffer[32 + 6] << 8) | raw_buffer[32 + 7];
		bool is_bright = (exposure != 0);
		
		if(verbose) Cout() << "  Frame found: exposure=" << exposure << ", is_bright=" << is_bright << ", balance=" << stats.bright_balance << "\n";

		bool skipped = false;
		if(mutex.TryEnter()) {
			const int MAX_BALANCE = 5;
			if (is_bright && stats.bright_balance >= MAX_BALANCE) skipped = true;
			else if (!is_bright && stats.bright_balance <= -MAX_BALANCE) skipped = true;
			
			if (skipped) {
				stats.other_errors++; // Skip counter
				if(verbose) Cout() << "    Skipped (balance=" << stats.bright_balance << ")\n";
			}
			mutex.Leave();
		} else {
			skipped = true;
			stats.mutex_fails++;
			if(verbose) Cout() << "    Skipped (mutex fail)\n";
		}
		
		if(!skipped) {
			int sj = 0;
			for (int i = 0; i < RAW_FRAME_SIZE; i += WMR_PACKET_SIZE) {
				int payload_offset = i + WMR_HEADER_SIZE;
				int n = WMR_PACKET_SIZE - WMR_HEADER_SIZE;
				if (i + WMR_PACKET_SIZE > RAW_FRAME_SIZE) n = RAW_FRAME_SIZE - payload_offset;
				if (n > 0) {
					memcpy(stripped_buffer.data() + sj, raw_buffer.data() + payload_offset, n);
					sj += n;
				}
			}
			
			if(sj >= STRIPPED_FRAME_SIZE) {
				ImageBuffer ib(1280, 481);
				const byte* src = stripped_buffer.data() + 26;
				uint64 sum = 0;
				byte min_p = 255, max_p = 0;
				RGBA* dst = ib;
				for(int p = 0; p < 1280 * 481; p++) {
					byte v = src[p];
					dst[p] = GrayColor(v);
					sum += v;
					if(v < min_p) min_p = v;
					if(v > max_p) max_p = v;
				}
				
				if(mutex.TryEnter()) {
					if(queue.GetCount() > 30) queue.Remove(0);
					CameraFrame& f = queue.Add();
					f.img = ib;
					f.is_bright = is_bright;
					f.exposure = exposure;
					
					stats.frame_count++;
					if(is_bright) { stats.bright_frames++; stats.bright_balance++; } 
					else { stats.dark_frames++; stats.bright_balance--; }
					stats.last_exposure = exposure;
					stats.avg_brightness = (double)sum / (1280 * 481);
					stats.min_pixel = min_p;
					stats.max_pixel = max_p;
					stats.handle_usecs = (int)(usecs() - start_usecs);
					mutex.Leave();
				} else {
					stats.mutex_fails++;
				}
			}
		}
		
		memmove(raw_buffer.data(), raw_buffer.data() + RAW_FRAME_SIZE, raw_buffer_ptr - RAW_FRAME_SIZE);
		raw_buffer_ptr -= RAW_FRAME_SIZE;
	}
}

NAMESPACE_HMD_END
