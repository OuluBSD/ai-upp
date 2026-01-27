#include "ExerciseEngine.h"

namespace Upp {

void ExerciseEngine::Update(double dt) {
	if (!running) return;
	current_time += dt;
	if (current_time > duration) {
		current_time = duration;
		running = false;
	}
}

ExerciseEngine::State ExerciseEngine::GetTargetState(double t) const {
	State s = { 0, 0 };
	if (nodes.IsEmpty()) return s;
	
	if (t <= nodes[0].time) {
		s.mode_val = (double)nodes[0].mode;
		s.pitch = nodes[0].pitch;
		return s;
	}
	
	if (t >= nodes.Top().time) {
		s.mode_val = (double)nodes.Top().mode;
		s.pitch = nodes.Top().pitch;
		return s;
	}
	
	for (int i = 0; i < nodes.GetCount() - 1; i++) {
		if (t >= nodes[i].time && t <= nodes[i+1].time) {
			double ratio = (t - nodes[i].time) / (nodes[i+1].time - nodes[i].time);
			s.mode_val = (double)nodes[i].mode + ratio * ((double)nodes[i+1].mode - (double)nodes[i].mode);
			s.pitch = nodes[i].pitch + ratio * (nodes[i+1].pitch - nodes[i].pitch);
			return s;
		}
	}
	
	return s;
}

double ExerciseEngine::GetAccuracy() const {
	State target = GetTargetState(current_time);
	if (target.pitch <= 0) return 0;
	
	double pitch_diff = abs(user_pitch - target.pitch) / target.pitch;
	double mode_diff = abs(user_mode_val - target.mode_val);
	
	double score = 100.0 * (1.0 - (pitch_diff + mode_diff) / 2.0);
	return max(0.0, score);
}

}
