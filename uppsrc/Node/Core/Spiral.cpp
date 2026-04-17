#include "Spiral.h"

namespace Upp {

namespace Node {

Vector<Pointf> SpiralPositions(int n, double spacing)
{
	Vector<Pointf> out;
	if(n <= 0) return out;
	out.Reserve(n);
	// Archimedean spiral: r = a * theta
	// We choose a so that arc length per turn ≈ spacing.
	// Arc length of Archimedean spiral per revolution ≈ pi * spacing (approx for large r).
	double a = spacing / (2.0 * M_PI);
	double theta = 0.0;
	for(int i = 0; i < n; i++) {
		double r = a * theta;
		out.Add(Pointf(r * cos(theta), r * sin(theta)));
		// Advance theta so that arc length step ≈ spacing
		// ds ≈ sqrt(r^2 + a^2) * dtheta ≈ r * dtheta for large r
		// For small r use a minimum step of 0.5 radians
		double dtheta = (r > 1e-6) ? spacing / r : 0.5;
		dtheta = clamp(dtheta, 0.05, 1.0);
		theta += dtheta;
	}
	return out;
}

Vector<Pointf> CirclePositions(int n, double spacing, double radius)
{
	Vector<Pointf> out;
	if(n <= 0) return out;
	out.Reserve(n);
	if(n == 1) { out.Add(Pointf(0, 0)); return out; }
	double r = (radius > 0.0) ? radius : (spacing * n) / (2.0 * M_PI);
	double step = 2.0 * M_PI / n;
	for(int i = 0; i < n; i++) {
		double a = i * step - M_PI / 2.0;  // start at top
		out.Add(Pointf(r * cos(a), r * sin(a)));
	}
	return out;
}

} // namespace Node

} // namespace Upp
