#include "GuboDemo.h"

using namespace Upp;

GuboDemo::GuboDemo() {
	SetFrameBox(CubfC(0, 0, 0, 640, 480, 480));  // Set initial size
	SetContentBox(CubfC(0, 0, 0, 640, 480, 480));
	Title("Gubo Demo");
}

void GuboDemo::Paint(Draw3& d) {
	Volf sz = GetFrameSize();

	// Draw a simple 3D scene

	// Draw a ground plane
	Volf ground = sz;
	ground.cy = 0.1;
	d.DrawBox(0, 0, 0, sz.cx, 0.1, sz.cz, Color(160, 221, 196));

	// Draw animated objects
	double time = ts.Seconds();
	float phase = fmod(time, animation_speed) * M_PI * 2;

	// Moving cube
	Point3f pos1(sin(phase) * 2, 1.0, cos(phase) * 2);
	d.DrawBox(pos1.x, pos1.y, pos1.z, 0.8, 0.8, 0.8, Color(255, 100, 100));

	// Bouncing cube
	Point3f pos2(2, fabs(sin(phase * 2)) * 3, 0);
	d.DrawBox(pos2.x, pos2.y, pos2.z, 0.8, 0.8, 0.8, Color(100, 255, 100));

	// Rotating cube
	float rotation = phase;
	Point3f pos3(-2, 2, 0);
	// For now, just draw a cube - actual rotation would require more complex transformations
	d.DrawBox(pos3.x, pos3.y, pos3.z, 0.8, 0.8, 0.8, Color(100, 100, 255));
}

// Entry point
GUI_APP_MAIN {
	try {
		GuboDemo demo;
		demo.OpenMain();
		demo.Run();
	}
	catch (const std::exception& e) {
		Exclamation("Exception: " + String(e.what()));
	}
	catch (...) {
		Exclamation("Unknown exception occurred");
	}
}