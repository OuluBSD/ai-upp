# Investigate Generic LIBUSB_TRANSFER_ERROR (Status 1) in SoftHMD Camera

## Context
The `SoftHMD` driver for Windows Mixed Reality (WMR) headsets is encountering persistent USB transfer errors when streaming camera data. 
- **Device:** HoloLens Sensors (VID: `045e`, PID: `0659`).
- **Connection:** Verified as **SuperSpeed (5Gbps)** USB 3.0 via `lsusb` and `script/fix_hmd_permissions.py`.
- **Symptoms:** 
    - The stream works intermittently (sometimes 20+ frames, sometimes 0).
    - `Camera::TransferCallback` reports `LIBUSB_TRANSFER_ERROR` (status `1`) repeatedly.
    - HID sensor data (IMU/Orientation) continues to arrive correctly even when video fails.
    - Error log: `USB Error: Transfer status 1`.

## Diagnostic Environment: WmrTest
The primary tool for testing is `upptst/WmrTest`.

### How to Build
Use the provided build script:
```bash
python3 script/build.py -mc 1 -j12 WmrTest
```

### How to Run
Run with the verbose flag to see frame-by-frame details and USB errors:
```bash
# Permissions must be fixed first via script/fix_hmd_permissions.py
./bin/WmrTest -v
```
To run a non-GUI dump for a specific duration:
```bash
./bin/WmrTest -v --test-dump 10
```

### Valgrind Analysis
Use Valgrind to detect memory corruption in the custom buffer reassembly logic or race conditions in the asynchronous callbacks.

**Memory Check (Memcheck):**
Checks for invalid pointer arithmetic, buffer overflows, and leaks in `HandleFrame`.
```bash
valgrind --tool=memcheck --leak-check=full ./bin/WmrTest -v --test-dump 5
```

**Thread Safety (Helgrind / DRD):**
Checks for race conditions on the `stats` and `queue` objects shared between the USB callback thread and the GUI/Process thread.
```bash
valgrind --tool=helgrind ./bin/WmrTest -v --test-dump 5
```
*Note: Expect slow performance and many warnings from the U++ core libraries; focus on SoftHMD/Camera.cpp stack traces.*

### What to Expect
- **Successful Run:** You will see `HandleFrame: size=616538` followed by `Frame found: exposure=...`. The GUI will display images from the headset's dual cameras.
- **Failure State:** The output will be flooded with `USB Error: Transfer status 1`. In the GUI sidebar, "Cam USB Errors" will ramp up rapidly while "Cam Frame Count" stays static.
- **Sensor Fusion:** Orientation updates (`[II] UpdateData: orient=...`) should continue regardless of camera state.

## Investigation Goals
1. **Understand Status Codes:**
    - **Status 1 (`LIBUSB_TRANSFER_ERROR`):** A generic failure occurred. This is a catch-all for "the hardware didn't respond correctly," "protocol error," or "controller error." It indicates the pipe might be broken or the device reset the connection.
    - **Status 3 (`LIBUSB_TRANSFER_CANCELLED`):** The transfer was cancelled. While normal during shutdown, seeing this during active streaming suggests the kernel or libusb is dropping packets, possibly due to a device disconnect/reconnect event or a race condition in transfer resubmission.
2. **Analyze Packet Size:** We are requesting `WMR_BULK_SIZE + 4096` bytes. Is the device sending packets that don't align with the endpoint's `wMaxPacketSize` (1024 for SS)?
3. **Check Endpoint Halt:** Does the error occur after a specific sequence? Do we need to clear the endpoint halt (`libusb_clear_halt`) *during* operation if an error occurs, not just at startup?
4. **Synchronous vs Asynchronous:** The original synchronous code (in `Camera::Open` thread loop) seemed less prone to this specific flood of errors, though it blocked. Is the async submission queue (`ASYNC_BUFFERS = 4`) overwhelming the controller?
5. **URB Status:** If possible, enable kernel-level USB tracing (`usbmon`) to see the actual URB status code returned by the Linux kernel (e.g., `-EPROTO`, `-EPIPE`).

## Actionable Steps

- [x] Modify `Camera.cpp` to attempt `libusb_clear_halt` on the video endpoint when status `1` is received, then resubmit.

- [x] Experiment with reducing `ASYNC_BUFFERS` to 2 or increasing to 8 to see if queue depth affects stability. (Selected 8 with 8MB buffer).

- [x] Verify if the "Command" endpoint needs a "keep-alive" or specific initialization sequence that is timing out, causing the video endpoint to stall. (Implemented 1s periodic gain refresh).

- [ ] Use `wireshark` or `tcpdump` with `usbmon` to capture the exact failure moment.
