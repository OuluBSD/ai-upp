#include "SoftHMD.h"

NAMESPACE_HMD_BEGIN

StereoTracker::StereoTracker() {
	orb.InitDefault();
	Reset();
}

void StereoTracker::Reset() {
	Upp::Mutex::Lock __(mutex);
	stats = StereoTrackerStats();
	position = vec3(0,0,0);
	orientation = Identity<quat>();
	write_index = 0;
	latest_index = -1;
	has_prev = false;
	octree_inited = false;
	frames[0].tracked_points.Clear();
	frames[0].tracked_triangles.Clear();
	frames[1].tracked_points.Clear();
	frames[1].tracked_triangles.Clear();
}

void StereoTracker::SetWmrDefaults() {
	Upp::Mutex::Lock __(mutex);
	uncam.SetAnglePixel(17.4932f, 153.022f, 175.333f, -25.7489f);
	uncam.SetEyeDistance(0.12f);
	uncam.SetYLevelHeight(10);
	uncam.SetEyeOutwardAngle(DEG2RAD(35.50f));
	uncam.SetDistanceLimit(128);
}

void StereoTracker::SetPointLimit(int limit) {
	Upp::Mutex::Lock __(mutex);
	point_limit = limit;
}

void StereoTracker::SetDistanceLimit(int limit) {
	Upp::Mutex::Lock __(mutex);
	uncam.SetDistanceLimit(limit);
}

void StereoTracker::SetUseBright(bool b) {
	Upp::Mutex::Lock __(mutex);
	use_bright = b;
}

void StereoTracker::SetUseDark(bool b) {
	Upp::Mutex::Lock __(mutex);
	use_dark = b;
}

void StereoTracker::SetSplitStereo(bool b) {
	Upp::Mutex::Lock __(mutex);
	split_stereo = b;
}

StereoTrackerStats StereoTracker::GetStats() const {
	Upp::Mutex::Lock __(mutex);
	return stats;
}

vec3 StereoTracker::GetPosition() const {
	Upp::Mutex::Lock __(mutex);
	return position;
}

quat StereoTracker::GetOrientation() const {
	Upp::Mutex::Lock __(mutex);
	return orientation;
}

bool StereoTracker::HasPose() const {
	Upp::Mutex::Lock __(mutex);
	return stats.has_pose;
}

const Octree* StereoTracker::GetPointcloud() const {
	Upp::Mutex::Lock __(mutex);
	if (latest_index < 0)
		return 0;
	return &frames[latest_index].otree;
}

bool StereoTracker::GetOverlay(StereoOverlay& out) const {
	Upp::Mutex::Lock __(mutex);
	if (latest_index < 0)
		return false;
	const UncameraFrame& frame = frames[latest_index];
	out.left_size = frame.l_dimg.GetResolution();
	out.right_size = frame.r_dimg.GetResolution();
	out.left_points.Clear();
	out.right_points.Clear();
	out.match_left.Clear();
	out.match_right.Clear();

	const Vector<Descriptor32>& left_desc = frame.l_dimg.GetDescriptors();
	out.left_points.SetCount(left_desc.GetCount());
	for (int i = 0; i < left_desc.GetCount(); i++) {
		const Descriptor32& d = left_desc[i];
		out.left_points[i] = vec2(d.x, d.y);
	}

	const Vector<Descriptor32>& right_desc = frame.r_dimg.GetDescriptors();
	out.right_points.SetCount(right_desc.GetCount());
	for (int i = 0; i < right_desc.GetCount(); i++) {
		const Descriptor32& d = right_desc[i];
		out.right_points[i] = vec2(d.x, d.y);
	}

	for (const HorizontalMatch& match : frame.horz_match) {
		if (!match.l || !match.r)
			continue;
		out.match_left.Add(vec2(match.l->x, match.l->y));
		out.match_right.Add(vec2(match.r->x, match.r->y));
	}
	return true;
}

bool StereoTracker::SplitStereo(const Image& frame, Image& left, Image& right) const {
	Size sz = frame.GetSize();
	if (sz.cx < 4 || sz.cy < 4)
		return false;
	if (!split_stereo) {
		left = frame;
		right = frame;
		return true;
	}
	if (sz.cx & 1)
		return false;
	Size half(sz.cx / 2, sz.cy);
	left = Crop(frame, RectC(0, 0, half.cx, half.cy));
	right = Crop(frame, RectC(half.cx, 0, half.cx, half.cy));
	return true;
}

void StereoTracker::InitOctrees() {
	if (octree_inited)
		return;
	frames[0].otree.Initialize(-3, 8);
	frames[1].otree.Initialize(-3, 8);
	octree_inited = true;
}

void StereoTracker::ClearFrame(UncameraFrame& frame) {
	frame.tracked_points.Clear();
	frame.tracked_triangles.Clear();
	frame.horz_match.Clear();
	frame.l_desc.Clear();
	frame.r_desc.Clear();
}

bool StereoTracker::PushFrame(const Image& frame, bool is_bright) {
	if (frame.IsEmpty())
		return false;

	Upp::Mutex::Lock __(mutex);
	stats.frame_count++;

	if ((is_bright && !use_bright) || (!is_bright && !use_dark)) {
		stats.skipped_frames++;
		return false;
	}

	Image left, right;
	if (!SplitStereo(frame, left, right)) {
		stats.skipped_frames++;
		return false;
	}

	InitOctrees();

	int64 start_usecs = usecs();
	UncameraFrame& cur = frames[write_index];
	UncameraFrame& prev = frames[1 - write_index];

	ClearFrame(cur);
	cur.l_img = left;
	cur.r_img = right;
	cur.l_dimg.SetResolution(left.GetSize());
	cur.r_dimg.SetResolution(right.GetSize());

	orb.SetInput(left);
	stats.last_left_keypoints = orb.DetectKeypoints(cur.l_dimg, point_limit);
	orb.SetInput(right);
	stats.last_right_keypoints = orb.DetectKeypoints(cur.r_dimg, point_limit);

	uncam.StageStereoKeypoints(prev, cur);
	uncam.ProcessTriangles(cur);
	if (has_prev) {
		uncam.SolveTransform(prev, cur);
		uncam.StageProcessTransform(prev, cur);
		position = cur.position;
		orientation = cur.orientation;
		stats.has_pose = true;
	}

	stats.last_tracked_points = cur.tracked_points.GetCount();
	stats.last_tracked_triangles = cur.tracked_triangles.GetCount();
	stats.last_process_usecs = (int)(usecs() - start_usecs);
	stats.processed_frames++;

	latest_index = write_index;
	write_index = 1 - write_index;
	has_prev = true;

	return true;
}

NAMESPACE_HMD_END
