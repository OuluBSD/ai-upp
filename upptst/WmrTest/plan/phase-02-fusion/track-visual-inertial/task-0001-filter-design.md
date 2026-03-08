Task 0001 - Filter design

Checklist
- Choose filter type: EKF/MSCKF vs fixed-lag smoother.
- Define state vector (pose, velocity, biases, scale).
- Define update triggers: stereo triangulation + temporal tracks.

Deliverables
- Fusion filter spec and initial parameters.

Filter choice (initial)
- Error-state EKF (ESKF) for real-time; MSCKF later if needed.

State vector
- Position (p), velocity (v), orientation (q).
- Accel/gyro biases (b_a, b_g).
- Optional scale + gravity vector (if visual scale is weak).

Update triggers
- IMU propagation at sensor rate.
- Stereo triangulation updates on matched feature tracks.
- Zero-velocity updates during detected stationary intervals.

Initial parameters
- Process noise: tuned from sensor specs; start conservative.
- Measurement noise: from reprojection error stats.
