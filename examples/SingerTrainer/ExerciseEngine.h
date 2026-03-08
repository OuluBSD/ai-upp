#ifndef _SingerTrainer_ExerciseEngine_h_
#define _SingerTrainer_ExerciseEngine_h_

#include <Core/Core.h>

namespace Upp {

struct ExerciseNode {
	double time;
	int    mode;
	double pitch; // Frequency in Hz
};

class ExerciseEngine {
	Vector<ExerciseNode> nodes;
	double current_time = 0;
	double duration = 0;
	bool running = false;
	
	double user_pitch = 0;
	double user_mode_val = 0; // Simulated user input

public:
	void Clear() { nodes.Clear(); duration = 0; current_time = 0; running = false; }
	void AddNode(double time, int mode, double pitch) {
		ExerciseNode& n = nodes.Add();
		n.time = time;
		n.mode = mode;
		n.pitch = pitch;
		if (time > duration) duration = time;
		Sort(nodes, [](const ExerciseNode& a, const ExerciseNode& b) { return a.time < b.time; });
	}
	
	void Start() { current_time = 0; running = true; }
	void Stop() { running = false; }
	
	void Update(double dt);
	
	double GetCurrentTime() const { return current_time; }
	double GetDuration() const { return duration; }
	bool   IsRunning() const { return running; }
	
	struct State {
		double mode_val;
		double pitch;
	};
	
	State GetTargetState(double t) const;
	double GetAccuracy() const;
	
	void SetUserInput(double pitch, double mode_val) { user_pitch = pitch; user_mode_val = mode_val; }
	
	const Vector<ExerciseNode>& GetNodes() const { return nodes; }
};

}

#endif
