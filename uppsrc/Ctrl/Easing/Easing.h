#ifndef _Ctrl_Easing_Easing_h_
#define _Ctrl_Easing_Easing_h_

#include <CtrlCore/CtrlCore.h>

namespace Upp {

// Linear easing curve
double EaseLinear(double t);

// Cubic easing curve - accelerating from zero velocity
double EaseInCubic(double t);

// Cubic easing curve - decelerating to zero velocity
double EaseOutCubic(double t);

// Cubic easing curve - acceleration until halfway, then deceleration
double EaseInOutCubic(double t);

// Tween class that drives a value from 0 to 1 over real time
class Tween {
	TimeCallback              tick;
	Function<double(double)>  curve;
	Function<void(double)>    on_step;
	Function<void()>          on_done;
	int                       start_ms = 0;
	int                       duration_ms = 0;
	bool                      running = false;

	void DoTick();

public:
	void Start(int duration_ms, Function<double(double)> curve,
	           Function<void(double)> on_step, Function<void()> on_done = Function<void()>());
	void Stop();
	bool IsRunning() const { return running; }
};

}

#endif
