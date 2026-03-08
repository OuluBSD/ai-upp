Task 0001 - IMU calibration and bias model

Checklist
- Define bias/scale model for accel and gyro.
- Define gravity alignment and initial orientation.
- Decide how biases are updated online.

Deliverables
- Calibration model spec and update strategy.

Calibration model
- Accel: scale + bias per axis (a_meas = S_a * (a_true + b_a)).
- Gyro: bias + scale (g_meas = S_g * (g_true + b_g)).
- Optional mag calibration (hard/soft iron) if magnetometer used.

Initialization
- Static window (1–2s) to estimate gravity vector and accel bias.
- Gyro bias from stationary mean.
- Initial orientation aligns IMU Z with gravity; yaw from magnetometer or visual.

Online update
- Bias random-walk in filter (low process noise).
- Re-calibrate biases on detected stationary intervals.
- Temperature compensation as optional linear model if sensor supports temp.
