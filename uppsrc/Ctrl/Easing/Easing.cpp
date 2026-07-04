#include "Easing.h"

namespace Upp {

double EaseLinear(double t)
{
	return t;
}

double EaseInCubic(double t)
{
	return t * t * t;
}

double EaseOutCubic(double t)
{
	return 1.0 - pow(1.0 - t, 3.0);
}

double EaseInOutCubic(double t)
{
	return t < 0.5 ? 4.0 * t * t * t : 1.0 - pow(-2.0 * t + 2.0, 3.0) / 2.0;
}

void Tween::Start(int duration_ms_, Function<double(double)> curve_,
                   Function<void(double)> on_step_, Function<void()> on_done_)
{
	duration_ms = duration_ms_;
	curve = curve_;
	on_step = on_step_;
	on_done = on_done_;
	start_ms = msecs();
	running = true;
	DoTick();
}

void Tween::Stop()
{
	tick.Kill();
	running = false;
}

void Tween::DoTick()
{
	int elapsed = msecs(start_ms);
	double t = min(1.0, double(elapsed) / duration_ms);
	if(on_step)
		on_step(curve(t));
	if(t >= 1.0) {
		running = false;
		if(on_done)
			on_done();
	}
	else {
		tick.Set(16, [this] { DoTick(); });
	}
}

}
