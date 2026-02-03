#include "IO.h"

NAMESPACE_UPP

namespace {

uint32 NextRand(uint32& state) {
	state = state * 1664525u + 1013904223u;
	return state;
}

float Randf(uint32& state, float minv, float maxv) {
	uint32 v = NextRand(state);
	float t = (float)(v & 0x00ffffff) / (float)0x01000000;
	return minv + (maxv - minv) * t;
}

vec3 ApplyPose(const PointcloudPose& pose, const vec3& p) {
	return VectorTransform(p, pose.orientation) + pose.position;
}

vec3 ApplyInversePose(const PointcloudPose& pose, const vec3& p) {
	vec3 rel = p - pose.position;
	quat inv = pose.orientation.GetConjugate();
	return VectorTransform(rel, inv);
}

ControllerPattern MakeCylinderPattern(const String& id, int dots_per_ring, float radius, float height, float phase) {
	ControllerPattern pattern;
	pattern.id = id;
	pattern.dots_local.SetCount(0);
	for (int ring = 0; ring < 2; ring++) {
		float z = ring == 0 ? 0.0f : height;
		for (int i = 0; i < dots_per_ring; i++) {
			float a = phase + (float)i * (2.0f * M_PIf / (float)dots_per_ring);
			vec3 p(cos(a) * radius, sin(a) * radius, z);
			pattern.dots_local.Add(p);
		}
	}
	return pattern;
}

} // namespace

SyntheticPointcloudState BuildSyntheticPointcloud(const SyntheticPointcloudConfig& cfg) {
	SyntheticPointcloudState out;
	out.reference.Clear();
	out.controllers.Clear();
	out.controller_poses_world.Clear();

	uint32 rng = (uint32)cfg.seed;
	out.reference.points.SetCount(cfg.random_points);
	out.reference.ids.SetCount(cfg.random_points);
	for (int i = 0; i < cfg.random_points; i++) {
		float x = Randf(rng, cfg.bounds_min[0], cfg.bounds_max[0]);
		float y = Randf(rng, cfg.bounds_min[1], cfg.bounds_max[1]);
		float z = Randf(rng, cfg.bounds_min[2], cfg.bounds_max[2]);
		out.reference.points[i] = vec3(x, y, z);
		out.reference.ids[i] = i;
	}

	ControllerPattern c0 = MakeCylinderPattern("controller_left",
		cfg.controller_dots_per_ring, cfg.controller_radius, cfg.controller_height, 0.2f);
	ControllerPattern c1 = MakeCylinderPattern("controller_right",
		cfg.controller_dots_per_ring, cfg.controller_radius, cfg.controller_height, 1.4f);
	out.controllers.Add(pick(c0));
	out.controllers.Add(pick(c1));

	PointcloudPose ctrl0;
	ctrl0.position = vec3(-0.4f, 0.1f, 0.6f);
	ctrl0.orientation = AxesQuat(0.2f, 0.1f, 0.0f);
	PointcloudPose ctrl1;
	ctrl1.position = vec3(0.5f, -0.05f, 0.7f);
	ctrl1.orientation = AxesQuat(-0.3f, 0.05f, 0.1f);
	out.controller_poses_world.Add(ctrl0);
	out.controller_poses_world.Add(ctrl1);

	out.hmd_pose_world.position = vec3(0.0f, 0.0f, 0.0f);
	out.hmd_pose_world.orientation = AxesQuat(0.0f, 0.0f, 0.0f);

	for (int ci = 0; ci < out.controllers.GetCount(); ci++) {
		const ControllerPattern& pat = out.controllers[ci];
		const PointcloudPose& pose = out.controller_poses_world[ci];
		for (const vec3& p : pat.dots_local) {
			out.reference.points.Add(ApplyPose(pose, p));
			out.reference.ids.Add(cfg.random_points + ci * 1000 + out.reference.points.GetCount());
		}
	}

	return out;
}

PointcloudObservation SimulateHmdObservation(const SyntheticPointcloudState& state,
                                             float max_range) {
	PointcloudObservation obs;
	obs.has_ground_truth = true;
	obs.ground_truth = state.hmd_pose_world;

	for (int i = 0; i < state.reference.points.GetCount(); i++) {
		vec3 cam = ApplyInversePose(state.hmd_pose_world, state.reference.points[i]);
		if (cam.GetLength() <= max_range) {
			obs.points.Add(cam);
			if (i < state.reference.ids.GetCount())
				obs.ids.Add(state.reference.ids[i]);
			else
				obs.ids.Add(i);
		}
	}

	return obs;
}

Vector<ControllerObservation> SimulateControllerObservations(const SyntheticPointcloudState& state) {
	Vector<ControllerObservation> out;
	out.SetCount(state.controllers.GetCount());
	for (int ci = 0; ci < state.controllers.GetCount(); ci++) {
		const ControllerPattern& pattern = state.controllers[ci];
		const PointcloudPose& ctrl_pose = state.controller_poses_world[ci];
		ControllerObservation obs;
		obs.has_ground_truth = true;
		obs.ground_truth = ctrl_pose;
		for (int i = 0; i < pattern.dots_local.GetCount(); i++) {
			vec3 world = ApplyPose(ctrl_pose, pattern.dots_local[i]);
			vec3 cam = ApplyInversePose(state.hmd_pose_world, world);
			obs.points.Add(cam);
			obs.ids.Add(i);
		}
		out[ci] = pick(obs);
	}
	return out;
}

bool RunSyntheticPointcloudSim(String& log, const SyntheticPointcloudConfig& cfg) {
	SyntheticPointcloudState state = BuildSyntheticPointcloud(cfg);
	PointcloudObservation obs = SimulateHmdObservation(state, cfg.max_range);

	PointcloudLocalizerStub localizer;
	PointcloudLocalizationResult loc = localizer.Locate(state.reference, obs);

	bool ok = loc.ok;
	log << "Synthetic sim: ref=" << state.reference.points.GetCount()
	    << " obs=" << obs.points.GetCount()
	    << " loc_ok=" << (int)loc.ok << "\n";

	Vector<ControllerObservation> ctrl_obs = SimulateControllerObservations(state);
	ControllerPatternTrackerStub tracker;
	ControllerFusionStub fusion;
	for (int i = 0; i < ctrl_obs.GetCount(); i++) {
		ControllerPoseResult pose = tracker.Track(state.controllers[i], ctrl_obs[i]);
		ControllerFusionSample sample;
		sample.has_orientation = true;
		sample.orientation = pose.pose.orientation;
		pose = fusion.Fuse(pose, sample);
		log << "  controller[" << i << "]=" << state.controllers[i].id
		    << " ok=" << (int)pose.ok << "\n";
		ok = ok && pose.ok;
	}

	return ok;
}

END_UPP_NAMESPACE
