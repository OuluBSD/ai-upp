#include "Routing.h"

namespace Upp {

namespace Node {

// ─── helpers ────────────────────────────────────────────────────────────────

static Pointf CubicBezier(const Pointf& p1, const Pointf& p2,
                          const Pointf& p3, const Pointf& p4, double t)
{
	double xa = p1.x + (p2.x - p1.x) * t;
	double ya = p1.y + (p2.y - p1.y) * t;
	double xb = p2.x + (p3.x - p2.x) * t;
	double yb = p2.y + (p3.y - p2.y) * t;
	double xc = p3.x + (p4.x - p3.x) * t;
	double yc = p3.y + (p4.y - p3.y) * t;
	double xm = xa + (xb - xa) * t;
	double ym = ya + (yb - ya) * t;
	double xn = xb + (xc - xb) * t;
	double yn = yb + (yc - yb) * t;
	return Pointf(xm + (xn - xm) * t, ym + (yn - ym) * t);
}

static void SampleBezier(Vector<Pointf>& path,
                         const Pointf& p1, const Pointf& p2,
                         const Pointf& p3, const Pointf& p4,
                         int n = 20)
{
	path.Reserve(path.GetCount() + n + 1);
	for (int i = 0; i <= n; i++)
		path.Add(CubicBezier(p1, p2, p3, p4, (double)i / n));
}

// Returns true if segment (a→b) intersects the AABB expanded by margin.
static bool SegmentHitsObstacle(Pointf a, Pointf b,
                                const Rectf& box, double margin = 8.0)
{
	Rectf r(box.left - margin, box.top - margin,
	        box.right + margin, box.bottom + margin);

	// Trivial accept: both endpoints inside
	auto inside = [&](Pointf p) {
		return p.x >= r.left && p.x <= r.right
		    && p.y >= r.top  && p.y <= r.bottom;
	};
	if (inside(a) || inside(b)) return true;

	// Trivial reject: both on same exterior side
	if (a.x < r.left   && b.x < r.left)   return false;
	if (a.x > r.right  && b.x > r.right)  return false;
	if (a.y < r.top    && b.y < r.top)    return false;
	if (a.y > r.bottom && b.y > r.bottom) return false;

	// Parametric Liang-Barsky style clipping
	double dx = b.x - a.x, dy = b.y - a.y;
	double t0 = 0.0, t1 = 1.0;
	auto clip = [&](double p, double q) -> bool {
		if (p == 0) return q >= 0;
		double t = q / p;
		if (p < 0) { if (t > t1) return false; if (t > t0) t0 = t; }
		else       { if (t < t0) return false; if (t < t1) t1 = t; }
		return true;
	};
	return clip(-dx, a.x - r.left)
	    && clip( dx, r.right - a.x)
	    && clip(-dy, a.y - r.top)
	    && clip( dy, r.bottom - a.y)
	    && t0 < t1;
}

// ─── Simple (original cubic bezier on dominant axis) ────────────────────────

static RouteResponse RouteSimple(const RouteRequest& req)
{
	RouteResponse res;
	Pointf p1 = req.source_pos, p4 = req.target_pos;
	double dx = max(abs(p1.x - p4.x) / 2.0, 10.0);
	double dy = max(abs(p1.y - p4.y) / 2.0, 10.0);
	Pointf p2 = p1, p3 = p4;
	if (abs(p1.x - p4.x) > abs(p1.y - p4.y)) {
		p2.x += (p4.x > p1.x ? dx : -dx);
		p3.x -= (p4.x > p1.x ? dx : -dx);
	} else {
		p2.y += (p4.y > p1.y ? dy : -dy);
		p3.y -= (p4.y > p1.y ? dy : -dy);
	}
	SampleBezier(res.path, p1, p2, p3, p4);
	return res;
}

// ─── Curved (S-curve: right-exit → left-entry, vertical jog over obstacles) ─

static RouteResponse RouteCurved(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;
	double hdx = abs(dst.x - src.x);
	double ctrl_x = max(60.0, hdx * 0.5);

	Pointf c1(src.x + ctrl_x, src.y);
	Pointf c2(dst.x - ctrl_x, dst.y);

	// Check whether the direct bezier arc hits any obstacle
	bool blocked = false;
	for (int i = 0; i <= 10; i++) {
		Pointf pt = CubicBezier(src, c1, c2, dst, i / 10.0);
		for (const Rectf& box : req.obstacles)
			if (SegmentHitsObstacle(pt, pt, box, 4.0)) { blocked = true; break; }
		if (blocked) break;
	}

	if (!blocked) {
		SampleBezier(res.path, src, c1, c2, dst, 24);
		return res;
	}

	// Build a jog: route src → mid_top → mid_bottom → dst
	// Find bounding union of all obstacles
	double obs_right = src.x, obs_left = dst.x;
	double obs_top   = min(src.y, dst.y);
	double obs_bot   = max(src.y, dst.y);
	for (const Rectf& box : req.obstacles) {
		obs_right = max(obs_right, box.right);
		obs_left  = min(obs_left,  box.left);
		obs_top   = min(obs_top,   (double)box.top);
		obs_bot   = max(obs_bot,   (double)box.bottom);
	}
	double jog_x  = obs_right + 24.0;      // route to the right of all obstacles
	double jog_y1 = src.y;
	double jog_y2 = dst.y;

	// Segment 1: src → jog waypoint 1 (src.y, jog_x)
	Pointf wp1(jog_x, src.y);
	Pointf wp2(jog_x, dst.y);
	Pointf s1c1(src.x + ctrl_x, src.y);
	Pointf s1c2(jog_x - 30, jog_y1);
	SampleBezier(res.path, src, s1c1, s1c2, wp1, 12);

	// Segment 2: vertical jog (wp1 → wp2)
	int vsteps = max(4, (int)(abs(wp2.y - wp1.y) / 20));
	for (int i = 1; i <= vsteps; i++)
		res.path.Add(Pointf(jog_x, wp1.y + (wp2.y - wp1.y) * i / vsteps));

	// Segment 3: jog → dst
	Pointf s3c1(jog_x + 30, jog_y2);
	Pointf s3c2(dst.x - ctrl_x, dst.y);
	SampleBezier(res.path, wp2, s3c1, s3c2, dst, 12);

	return res;
}

// ─── Schematic (right-angle orthogonal routing) ─────────────────────────────

static RouteResponse RouteSchematic(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;

	double mid_x = (src.x + dst.x) * 0.5;

	// If source is to the left of destination: simple L/Z shape
	// src → (mid_x, src.y) → (mid_x, dst.y) → dst
	// with small 4px corner radius approximated by 2 extra points
	const double r = 4.0;
	res.path.Add(src);
	if (abs(src.y - dst.y) < 2.0) {
		// Same row — straight line
		res.path.Add(dst);
	} else {
		res.path.Add(Pointf(mid_x - r, src.y));
		res.path.Add(Pointf(mid_x,     src.y + (dst.y > src.y ? r : -r)));
		res.path.Add(Pointf(mid_x,     dst.y - (dst.y > src.y ? r : -r)));
		res.path.Add(Pointf(mid_x + r, dst.y));
		res.path.Add(dst);
	}
	return res;
}

// ─── Realistic (catenary cable sag + sway) ───────────────────────────────────

static RouteResponse RouteRealistic(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;
	int    n   = 32;
	double dist = sqrt((dst.x - src.x) * (dst.x - src.x) +
	                   (dst.y - src.y) * (dst.y - src.y));
	double slack  = max(0.0, dist * 0.18);  // extra cable length
	double sag    = slack * 0.8;            // downward parabolic sag

	// Perpendicular sway from animation phase
	double phase = req.anim_phase * 2.0 * M_PI;
	double sway  = sin(phase) * dist * 0.02;

	// Unit vectors along cable and perpendicular (rotated 90°)
	double len  = dist > 0.001 ? dist : 0.001;
	double along_x = (dst.x - src.x) / len;
	double along_y = (dst.y - src.y) / len;
	double perp_x  = -along_y;
	double perp_y  =  along_x;

	res.path.Reserve(n + 1);
	for (int i = 0; i <= n; i++) {
		double t    = (double)i / n;
		// Linear interpolation along chord
		double cx   = src.x + (dst.x - src.x) * t;
		double cy   = src.y + (dst.y - src.y) * t;
		// Parabolic sag: 0 at endpoints, max at midpoint
		double drop = sag * 4.0 * t * (1.0 - t);
		// Sway oscillation (perpendicular to cable direction)
		double sw   = sway * sin(t * M_PI);
		res.path.Add(Pointf(cx + drop * 0.0  + sw * perp_x,
		                    cy + drop        + sw * perp_y));
	}
	return res;
}

// ─── Dispatch ───────────────────────────────────────────────────────────────

RouteResponse BezierRoutingPolicy::Route(const RouteRequest& req)
{
	switch (req.style) {
	case EdgeStyle::Simple:    return RouteSimple(req);
	case EdgeStyle::Curved:    return RouteCurved(req);
	case EdgeStyle::Schematic: return RouteSchematic(req);
	case EdgeStyle::Realistic: return RouteRealistic(req);
	default:                   return RouteSimple(req);
	}
}

} // namespace Node

} // namespace Upp
