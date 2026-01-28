#include "SoftHMD.h"

NAMESPACE_HMD_BEGIN

SoftHmdVisualTracker::SoftHmdVisualTracker() {
	tracker.SetWmrDefaults();
}

void SoftHmdVisualTracker::Reset() {
	tracker.Reset();
	has_state = false;
	state = FusionState();
}

void SoftHmdVisualTracker::SetUseBright(bool b) {
	tracker.SetUseBright(b);
}

void SoftHmdVisualTracker::SetUseDark(bool b) {
	tracker.SetUseDark(b);
}

void SoftHmdVisualTracker::SetSplitStereo(bool b) {
	tracker.SetSplitStereo(b);
}

void SoftHmdVisualTracker::SetPointLimit(int limit) {
	tracker.SetPointLimit(limit);
}

void SoftHmdVisualTracker::SetDistanceLimit(int limit) {
	tracker.SetDistanceLimit(limit);
}

StereoTrackerStats SoftHmdVisualTracker::GetStats() const {
	return tracker.GetStats();
}

const Octree* SoftHmdVisualTracker::GetPointcloud() const {
	return tracker.GetPointcloud();
}

bool SoftHmdVisualTracker::HasPose() const {
	return tracker.HasPose();
}

vec3 SoftHmdVisualTracker::GetPosition() const {
	return tracker.GetPosition();
}

quat SoftHmdVisualTracker::GetOrientation() const {
	return tracker.GetOrientation();
}

bool SoftHmdVisualTracker::DecodeFrame(const VisualFrame& frame, Image& out) const {
	if (!frame.data || frame.width <= 0 || frame.height <= 0 || frame.stride <= 0)
		return false;
	
	Size sz(frame.width, frame.height);
	ImageBuffer ib(sz);
	
	switch (frame.format) {
		case GEOM_EVENT_CAM_LUMA8: {
			for (int y = 0; y < sz.cy; y++) {
				const byte* src = frame.data + y * frame.stride;
				RGBA* dst = ib[y];
				for (int x = 0; x < sz.cx; x++) {
					byte v = src[x];
					dst[x] = RGBA(v, v, v, 255);
				}
			}
			break;
		}
		case GEOM_EVENT_CAM_BGRA8: {
			for (int y = 0; y < sz.cy; y++) {
				const byte* src = frame.data + y * frame.stride;
				RGBA* dst = ib[y];
				for (int x = 0; x < sz.cx; x++) {
					const byte* p = src + x * 4;
					dst[x] = RGBA(p[2], p[1], p[0], p[3]);
				}
			}
			break;
		}
		case GEOM_EVENT_CAM_RGBA8: {
			for (int y = 0; y < sz.cy; y++) {
				const byte* src = frame.data + y * frame.stride;
				RGBA* dst = ib[y];
				for (int x = 0; x < sz.cx; x++) {
					const byte* p = src + x * 4;
					dst[x] = RGBA(p[0], p[1], p[2], p[3]);
				}
			}
			break;
		}
		default:
			return false;
	}
	
	out = ib;
	return true;
}

void SoftHmdVisualTracker::PutFrame(const VisualFrame& frame) {
	Image img;
	if (!DecodeFrame(frame, img))
		return;
	
	bool is_bright = (frame.flags & VIS_FRAME_BRIGHT) != 0;
	if ((frame.flags & (VIS_FRAME_BRIGHT | VIS_FRAME_DARK)) == 0)
		is_bright = true;
	
	if (!tracker.PushFrame(img, is_bright))
		return;
	
	if (tracker.HasPose()) {
		state.timestamp_us = frame.timestamp_us;
		state.position = tracker.GetPosition();
		state.orientation = tracker.GetOrientation();
		has_state = true;
	}
}

bool SoftHmdVisualTracker::GetState(FusionState& out) const {
	if (!has_state)
		return false;
	out = state;
	return true;
}


void SoftHmdImuTracker::Reset() {
	has_sample = false;
	last_sample = ImuSample();
}

void SoftHmdImuTracker::PutSample(const ImuSample& sample) {
	last_sample = sample;
	has_sample = true;
}


void SoftHmdFusion::Reset() {
	visual.Reset();
	imu.Reset();
	has_state = false;
	state = FusionState();
}

void SoftHmdFusion::PutVisual(const VisualFrame& frame) {
	visual.PutFrame(frame);
	FusionState st;
	if (visual.GetState(st)) {
		state = st;
		has_state = true;
	}
}

void SoftHmdFusion::PutImu(const ImuSample& sample) {
	imu.PutSample(sample);
}

bool SoftHmdFusion::GetState(FusionState& out) const {
	if (!has_state)
		return false;
	out = state;
	return true;
}

NAMESPACE_HMD_END
