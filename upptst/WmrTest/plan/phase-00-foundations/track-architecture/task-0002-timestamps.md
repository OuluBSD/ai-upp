Task 0002 - Timestamp model and sensor packet format

Checklist
- Decide timestamp source for camera packets (device vs host assigned).
- Define monotonic clock used for fusion.
- Define a packet struct for camera + IMU with timestamp and sequence id.

Timestamp model (proposal)
- Use a single monotonic timebase (usecs) from host (`usecs()`).
- If device timestamps are exposed later, store both and maintain an offset estimate.
- All sensor packets should include:
  - timestamp_usecs (host)
  - sequence id (per sensor stream)
  - source id (camera/imu/etc)

Camera packets
- If libusb has no timestamp, assign on transfer completion.
- If camera frames are assembled from multiple transfers, timestamp the frame at first byte or completion; pick one and document.
  - Prefer "first byte" for lower latency, but capture both if available.

IMU packets
- If IMU data is polled, timestamp on read.
- If IMU data comes with device time, include both and compute host offset.

Proposed structs (Geometry-level)
- SensorTimestamp { int64 usecs; int32 seq; int32 source; }
- CameraFramePacket { SensorTimestamp ts; Image img; bool is_bright; }
- ImuSamplePacket { SensorTimestamp ts; vec3 accel; vec3 gyro; vec3 mag; }

Deliverables
- A stable timestamp spec and draft struct definitions.
