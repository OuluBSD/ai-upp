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
	active_transfers = 0;
	usb_ctx = NULL;
	usb_handle = NULL;
	async_buffers = 8;
	transfer_timeout_ms = 0;
	raw_buffer.assign(8000000, 0); // 8MB buffer
	raw_buffer_ptr = 0;
	last_halt_clear_usecs = 0;
	transfers.clear();
	skip_streak_bright = 0;
	skip_streak_dark = 0;
}

Camera::~Camera()
{
	Close();
}

void LIBUSB_CALL Camera::TransferCallback(struct libusb_transfer* xfer)
{
	Transfer* t = (Transfer*)xfer->user_data;
	Camera* cam = t->camera;
	const int status = (int)xfer->status;
	const bool locked = cam->mutex.TryEnter();
	if(locked && status >= 0 && status <= LIBUSB_TRANSFER_OVERFLOW)
		cam->stats.status_counts[status]++;
	
	if(xfer->status == LIBUSB_TRANSFER_COMPLETED) {
		if(xfer->actual_length > 0) {
			if(locked) {
				cam->stats.last_transferred = xfer->actual_length;
				if(xfer->actual_length < cam->stats.min_transferred)
					cam->stats.min_transferred = xfer->actual_length;
				if(xfer->actual_length > cam->stats.max_transferred)
					cam->stats.max_transferred = xfer->actual_length;
				cam->stats.last_error = 0;
				cam->stats.last_r = 0;
				cam->AppendRaw(xfer->buffer, xfer->actual_length);
			}
		} else {
			if(cam->verbose) Cout() << "USB: Zero-byte transfer completed\n";
		}
	}
	else {
		Cout() << "USB Error: Transfer status " << status << "\n";
		if(locked) {
			cam->stats.last_error = xfer->status;
			cam->stats.last_r = xfer->status;
			cam->stats.usb_errors++;
			if(xfer->status == LIBUSB_TRANSFER_NO_DEVICE) cam->stats.no_device_errors++;
			if (xfer->status == LIBUSB_TRANSFER_TIMED_OUT) cam->stats.timeout_errors++;
			else if (xfer->status == LIBUSB_TRANSFER_OVERFLOW) cam->stats.overflow_errors++;
			else cam->stats.other_errors++;
		}
	}
	
	if(cam->usb_flag.IsRunning()) {
		if(xfer->status == LIBUSB_TRANSFER_ERROR || xfer->status == LIBUSB_TRANSFER_STALL) {
			bool should_clear = true;
			if(locked) {
				const int64 now = usecs();
				// Limit clear_halt frequency to once per 500ms per error to avoid spamming the controller
				if(cam->last_halt_clear_usecs != 0 && now - cam->last_halt_clear_usecs < 500000)
					should_clear = false;
				if(should_clear)
					cam->last_halt_clear_usecs = now;
			}
			if(should_clear) {
				int ch = libusb_clear_halt(cam->usb_handle, WMR_VIDEO_ENDPOINT);
				if(locked) {
					cam->stats.halt_clear_attempts++;
					if(ch != 0)
						cam->stats.halt_clear_failures++;
				}
				if(ch != 0)
					Cout() << "USB Error: Failed to clear halt: " << libusb_error_name(ch) << " (" << ch << ")\n";
				else
					Cout() << "USB: Successfully cleared halt on video endpoint\n";
			}
		}
		bool allow_resubmit = true;
		if(xfer->status == LIBUSB_TRANSFER_NO_DEVICE || xfer->status == LIBUSB_TRANSFER_CANCELLED)
			allow_resubmit = false;
		if(allow_resubmit) {
			int r = libusb_submit_transfer(xfer);
			if (r != 0) {
				Cout() << "USB Error: Failed to resubmit transfer: " << libusb_error_name(r) << " (" << r << ")\n";
				cam->active_transfers--;
				if(locked) {
					cam->stats.usb_errors++;
					cam->stats.last_r = r;
					cam->stats.resubmit_failures++;
				}
			}
		} else {
			cam->active_transfers--;
			if(locked) {
				cam->stats.resubmit_skips++;
			}
		}
	}
	
	if(locked)
		cam->mutex.Leave();
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
	if(async_buffers <= 0) async_buffers = 1;
	if(mutex.TryEnter()) {
		stats.async_buffers = async_buffers;
		stats.transfer_timeout_ms = transfer_timeout_ms;
		mutex.Leave();
	}
	
	if(libusb_init(&usb_ctx) != 0) return false;
	
	usb_handle = libusb_open_device_with_vid_pid(usb_ctx, WMR_VID, WMR_PID);
	if(!usb_handle) {
		libusb_exit(usb_ctx);
		usb_ctx = NULL;
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
	
	int effective_timeout_ms = transfer_timeout_ms;
	if(effective_timeout_ms <= 0)
		effective_timeout_ms = 1000;
	if(mutex.TryEnter()) {
		stats.transfer_timeout_ms = effective_timeout_ms;
		mutex.Leave();
	}
	
	transfers.assign(async_buffers, Transfer{});
	for(int i = 0; i < async_buffers; i++) {
		transfers[i].camera = this;
		transfers[i].buffer = (byte*)malloc(WMR_BULK_SIZE + 4096);
		transfers[i].libusb_xfer = libusb_alloc_transfer(0);
		libusb_fill_bulk_transfer(transfers[i].libusb_xfer, usb_handle, WMR_VIDEO_ENDPOINT,
		                          transfers[i].buffer, WMR_BULK_SIZE + 4096,
		                          TransferCallback, &transfers[i], effective_timeout_ms);
		libusb_submit_transfer(transfers[i].libusb_xfer);
	}
	active_transfers = async_buffers;

	usb_flag.Start();
	process_flag.Start();
	
	usb_thread.Start(THISBACK(Process));
	process_thread.Start(THISBACK(ProcessFrames));
	opened = true;
	
	return true;
}

void HMD_APIENTRYDLL Camera::Close()
{
	if(!opened) return;
	
	process_flag.Stop();
	usb_flag.Stop();
	
	for(int i = 0; i < transfers.size(); i++) {
		if(transfers[i].libusb_xfer)
			libusb_cancel_transfer(transfers[i].libusb_xfer);
	}
	
	int attempts = 0;
	while(active_transfers > 0 && attempts < 200) {
		Upp::Sleep(10);
		attempts++;
	}
	
	usb_thread.Wait();
	process_thread.Wait();
	
	if(usb_handle) {
		wmr_camera_set_active(usb_handle, false);
		for(int i = 3; i <= 4; i++) {
			libusb_release_interface(usb_handle, i);
		}
		libusb_close(usb_handle);
	}
	
	libusb_exit(usb_ctx);
	usb_ctx = NULL;
	
	for(int i = 0; i < transfers.size(); i++) {
		if(transfers[i].libusb_xfer)
			libusb_free_transfer(transfers[i].libusb_xfer);
		if(transfers[i].buffer)
			free(transfers[i].buffer);
		transfers[i].libusb_xfer = NULL;
		transfers[i].buffer = NULL;
	}
	transfers.clear();
	
	usb_handle = NULL;
	opened = false;
}

void Camera::Process()
{
	struct timeval tv = { 0, 10000 };
	while(usb_flag.IsRunning()) {
		libusb_handle_events_timeout_completed(usb_ctx, &tv, NULL);
	}
	usb_flag.SetStopped();
}

void Camera::ProcessFrames()
{
	while(process_flag.IsRunning()) {
		if(!ProcessRawFrames())
			Upp::Sleep(1);
	}
	process_flag.SetStopped();
}

void Camera::AppendRaw(const byte* buffer, int size)
{
	Upp::Mutex::Lock __(raw_mutex);
	AppendRawLocked(buffer, size);
}

void Camera::AppendRawLocked(const byte* buffer, int size)
{
	if (size <= 0)
		return;
	int capacity = (int)raw_buffer.size();
	if (size > capacity) {
		if (verbose) Cout() << "Raw append size exceeds capacity, taking tail\n";
		buffer += (size - capacity);
		size = capacity;
	}
	
	if (raw_buffer_ptr < 0)
		raw_buffer_ptr = 0;

	if(raw_buffer_ptr + size > capacity) {
		if(verbose) Cout() << "Buffer overflow in AppendRawLocked, shifting data\n";
		int overflow = (raw_buffer_ptr + size) - capacity;
		int keep = raw_buffer_ptr - overflow;
		if (keep > 0) {
			memmove(raw_buffer.data(), raw_buffer.data() + overflow, keep);
			raw_buffer_ptr = keep;
		} else {
			raw_buffer_ptr = 0;
		}
	}
	memcpy(raw_buffer.data() + raw_buffer_ptr, buffer, size);
	raw_buffer_ptr += size;
}

bool Camera::ProcessRawFrames()
{
	int64 start_usecs = usecs();
	process_buffer.clear();
	int local_size = 0;
	{
		Upp::Mutex::Lock __(raw_mutex);
		if (raw_buffer_ptr < 0)
			raw_buffer_ptr = 0;
		if (raw_buffer_ptr > (int)raw_buffer.size())
			raw_buffer_ptr = (int)raw_buffer.size();
		if(raw_buffer_ptr > 0) {
			process_buffer.assign(raw_buffer.data(), raw_buffer.data() + raw_buffer_ptr);
			local_size = raw_buffer_ptr;
			raw_buffer_ptr = 0;
		}
	}
	if(local_size == 0)
		return false;

	const int RAW_FRAME_SIZE = 616538;
	if(verbose && local_size >= RAW_FRAME_SIZE)
		Cout() << "HandleFrame: size=" << local_size << ", raw_ptr=" << local_size << "\n";

	const int STRIPPED_FRAME_SIZE = 615706;
	static thread_local std::vector<byte> stripped;
	if((int)stripped.size() < STRIPPED_FRAME_SIZE)
		stripped.assign(STRIPPED_FRAME_SIZE, 0);
	
	int local_ptr = local_size;
	byte* local_data = process_buffer.data();
	bool processed = false;
	while(local_ptr >= RAW_FRAME_SIZE) {
		uint16 exposure = (local_data[32 + 6] << 8) | local_data[32 + 7];
		bool is_bright = (exposure != 0);
		
		if(verbose) Cout() << "  Frame found: exposure=" << exposure << ", is_bright=" << is_bright << ", balance=" << stats.bright_balance << "\n";

		bool skipped = false;
		bool balance_skip = false;
		{
			Upp::Mutex::Lock __(mutex);
			const int MAX_BALANCE = 5;
			if (is_bright && stats.bright_balance >= MAX_BALANCE) { skipped = true; balance_skip = true; }
			else if (!is_bright && stats.bright_balance <= -MAX_BALANCE) { skipped = true; balance_skip = true; }
			
			if (skipped) {
				stats.other_errors++; // Skip counter
				if(verbose) Cout() << "    Skipped (balance=" << stats.bright_balance << ")\n";
			}
		}
		
		if(balance_skip) {
			const int MAX_SKIP_STREAK = 5;
			if(is_bright) {
				skip_streak_bright++;
				if(skip_streak_bright >= MAX_SKIP_STREAK) {
					skip_streak_bright = 0;
					skipped = false;
					{ Upp::Mutex::Lock __(mutex); stats.bright_balance = 0; }
				}
			} else {
				skip_streak_dark++;
				if(skip_streak_dark >= MAX_SKIP_STREAK) {
					skip_streak_dark = 0;
					skipped = false;
					{ Upp::Mutex::Lock __(mutex); stats.bright_balance = 0; }
				}
			}
		} else {
			skip_streak_bright = 0;
			skip_streak_dark = 0;
		}
		
		if(!skipped) {
			processed = true;
			int sj = 0;
			for (int i = 0; i < RAW_FRAME_SIZE; i += WMR_PACKET_SIZE) {
				int payload_offset = i + WMR_HEADER_SIZE;
				int n = WMR_PACKET_SIZE - WMR_HEADER_SIZE;
				if (i + WMR_PACKET_SIZE > RAW_FRAME_SIZE) n = RAW_FRAME_SIZE - payload_offset;
				if(sj + n > STRIPPED_FRAME_SIZE)
					n = STRIPPED_FRAME_SIZE - sj;
				if (n > 0) {
					memcpy(stripped.data() + sj, local_data + payload_offset, n);
					sj += n;
				}
			}
			
			if(sj >= STRIPPED_FRAME_SIZE) {
				ImageBuffer ib(1280, 481);
				const byte* src = stripped.data() + 26;
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
				
				{
					Upp::Mutex::Lock __(mutex);
					if(queue.GetCount() >= 2) queue.Remove(0, queue.GetCount() - 1);
					CameraFrame& f = queue.Add();
					f.img = ib;
					f.is_bright = is_bright;
					f.exposure = exposure;
					
					stats.frame_count++;
					if(is_bright) { stats.bright_frames++; stats.bright_balance++; } 
					else { stats.dark_frames++; stats.bright_balance--; }
					stats.bright_balance = Clamp(stats.bright_balance, -20, 20);
					stats.last_exposure = exposure;
					stats.avg_brightness = (double)sum / (1280 * 481);
					stats.min_pixel = min_p;
					stats.max_pixel = max_p;
					stats.handle_usecs = (int)(usecs() - start_usecs);
				}
			}
		}
		
		memmove(local_data, local_data + RAW_FRAME_SIZE, local_ptr - RAW_FRAME_SIZE);
		local_ptr -= RAW_FRAME_SIZE;
	}
	
	if(local_ptr > 0) {
		Upp::Mutex::Lock __(raw_mutex);
		
		// We need to prepend 'local_ptr' bytes (leftovers) to whatever new data 
		// has been appended to raw_buffer while we were processing.
		
		int new_data_size = raw_buffer_ptr;
		if (new_data_size < 0) new_data_size = 0;
		int capacity = (int)raw_buffer.size();

		// Safety clamp for leftovers
		if (local_ptr > capacity)
			local_ptr = capacity;
		
		// Check total size
		if (local_ptr + new_data_size > capacity) {
			if (verbose) Cout() << "Buffer full/overflow in ProcessRawFrames prepending leftovers.\n";
			// Prioritize stream continuity: keep leftovers, truncate new data if needed
			int space_for_new = capacity - local_ptr;
			if (new_data_size > space_for_new)
				new_data_size = space_for_new;
		}

		// Shift new data to the right to make room for leftovers
		if (new_data_size > 0) {
			memmove(raw_buffer.data() + local_ptr, raw_buffer.data(), new_data_size);
		}
		
		// Copy leftovers to the front
		memcpy(raw_buffer.data(), local_data, local_ptr);
		
		// Update size
		raw_buffer_ptr = local_ptr + new_data_size;
	}
	
	return processed;
}

NAMESPACE_HMD_END
