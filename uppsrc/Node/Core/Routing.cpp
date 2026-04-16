#include "Routing.h"

namespace Upp {

namespace Node {

// ─── Low-level geometry helpers ─────────────────────────────────────────────

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

// Liang-Barsky segment-AABB intersection, with optional margin expansion.
static bool SegmentHitsObstacle(Pointf a, Pointf b,
                                const Rectf& box, double margin = 8.0)
{
	Rectf r(box.left - margin, box.top - margin,
	        box.right + margin, box.bottom + margin);
	auto inside = [&](Pointf p) {
		return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
	};
	if (inside(a) || inside(b)) return true;
	if (a.x < r.left   && b.x < r.left)   return false;
	if (a.x > r.right  && b.x > r.right)  return false;
	if (a.y < r.top    && b.y < r.top)    return false;
	if (a.y > r.bottom && b.y > r.bottom) return false;
	double dx = b.x - a.x, dy = b.y - a.y;
	double t0 = 0.0, t1 = 1.0;
	auto clip = [&](double p, double q) -> bool {
		if (p == 0) return q >= 0;
		double t = q / p;
		if (p < 0) { if (t > t1) return false; if (t > t0) t0 = t; }
		else       { if (t < t0) return false; if (t < t1) t1 = t; }
		return true;
	};
	return clip(-dx, a.x - r.left) && clip(dx, r.right - a.x)
	    && clip(-dy, a.y - r.top)  && clip(dy, r.bottom - a.y) && t0 < t1;
}

static bool PathHitsAny(const Pointf& a, const Pointf& b,
                        const Vector<Rectf>& obstacles, double margin = 6.0)
{
	for (const Rectf& box : obstacles)
		if (SegmentHitsObstacle(a, b, box, margin)) return true;
	return false;
}

static double SegLen(Pointf a, Pointf b) {
	double dx = b.x-a.x, dy = b.y-a.y;
	return sqrt(dx*dx + dy*dy);
}

// ─── Visibility-graph shortest path ─────────────────────────────────────────
// Builds a small waypoint graph from obstacle corners + src/dst,
// finds shortest non-blocked path using Dijkstra.

static Vector<Pointf> VisibilityShortestPath(
    Pointf src, Pointf dst,
    const Vector<Rectf>& obstacles, double margin = 8.0)
{
	// Candidate waypoints: obstacle corners (expanded by margin + gap)
	const double GAP = margin + 4.0;
	Vector<Pointf> pts;
	pts.Add(src);
	pts.Add(dst);
	for (const Rectf& r : obstacles) {
		pts.Add(Pointf(r.left  - GAP, r.top    - GAP));
		pts.Add(Pointf(r.right + GAP, r.top    - GAP));
		pts.Add(Pointf(r.left  - GAP, r.bottom + GAP));
		pts.Add(Pointf(r.right + GAP, r.bottom + GAP));
	}

	int n = pts.GetCount();
	// Adjacency: cost = Euclidean if segment is clear
	Vector<double> dist(n, 1e18);
	Vector<int>    prev(n, -1);
	dist[0] = 0;

	// Simple O(n²) Dijkstra (n is small: ~4*obstacles+2)
	Vector<bool> visited(n, false);
	for (int iter = 0; iter < n; iter++) {
		// Pick unvisited node with lowest dist
		int u = -1;
		for (int i = 0; i < n; i++)
			if (!visited[i] && (u < 0 || dist[i] < dist[u])) u = i;
		if (u < 0 || dist[u] >= 1e17) break;
		visited[u] = true;
		for (int v = 0; v < n; v++) {
			if (visited[v]) continue;
			if (PathHitsAny(pts[u], pts[v], obstacles, margin)) continue;
			double d = dist[u] + SegLen(pts[u], pts[v]);
			if (d < dist[v]) { dist[v] = d; prev[v] = u; }
		}
	}

	// Reconstruct path (index 1 = dst)
	Vector<Pointf> path;
	if (dist[1] >= 1e17) {
		// No clear path found — just use direct line
		path.Add(src); path.Add(dst);
		return path;
	}
	Vector<int> idx;
	for (int cur = 1; cur >= 0; cur = prev[cur]) {
		idx.Add(cur);
		if (cur == 0) break;
		if (prev[cur] < 0) break;
	}
	for (int i = idx.GetCount() - 1; i >= 0; i--)
		path.Add(pts[idx[i]]);
	return path;
}

// Round a polyline: replace each interior vertex with a bezier arc of given radius.
static void RoundedPolyline(Vector<Pointf>& out, const Vector<Pointf>& pts, double radius)
{
	if (pts.GetCount() < 2) return;
	out.Add(pts[0]);
	for (int i = 1; i < pts.GetCount() - 1; i++) {
		Pointf prev = pts[i-1], cur = pts[i], next = pts[i+1];
		double d1 = SegLen(prev, cur), d2 = SegLen(cur, next);
		double r = min(radius, min(d1, d2) * 0.4);
		if (r < 1.0) { out.Add(cur); continue; }
		// Entry point: r before corner on incoming segment
		double t1 = (d1 > 0) ? r / d1 : 0;
		Pointf e((1-t1)*prev.x + t1*cur.x, (1-t1)*prev.y + t1*cur.y);
		// On-ramp point (at corner - r from prev side, interpolate toward next)
		double t2 = (d2 > 0) ? r / d2 : 0;
		Pointf x(cur.x + t2*(next.x-cur.x), cur.y + t2*(next.y-cur.y));
		// Quadratic bezier: e → cur → x (3 samples enough for small arcs)
		const int ARC = 6;
		for (int j = 0; j <= ARC; j++) {
			double t = (double)j / ARC;
			double bx = (1-t)*(1-t)*e.x + 2*(1-t)*t*cur.x + t*t*x.x;
			double by = (1-t)*(1-t)*e.y + 2*(1-t)*t*cur.y + t*t*x.y;
			out.Add(Pointf(bx, by));
		}
	}
	out.Add(pts[pts.GetCount() - 1]);
}

// ─── Orthogonal channel router for Schematic ─────────────────────────────────
// Routes H/V segments through corridors between obstacles.
// Strategy:
//   1. Try a direct mid-X Z-route if the 3 segments are all clear.
//   2. Otherwise pick the best X (from a set of candidates) that keeps segments clear.
//   3. If even that fails (very crowded), fall back to a multi-hop route around the union
//      of blocking obstacles.

static Vector<Pointf> OrthoRoute(Pointf src, Pointf dst,
                                 const Vector<Rectf>& obstacles)
{
	const double MARGIN = 6.0;
	const double STEP   = 20.0; // candidate vertical channel spacing

	// Build candidate mid-X values: midpoint, obstacle edges with padding, plus extremes
	Vector<double> cands;
	cands.Add((src.x + dst.x) * 0.5);
	for (const Rectf& r : obstacles) {
		cands.Add(r.left  - MARGIN * 2);
		cands.Add(r.right + MARGIN * 2);
	}
	// Also try X positions stepping outward from mid
	double mid = (src.x + dst.x) * 0.5;
	for (double delta = 0; delta < 800; delta += STEP) {
		cands.Add(mid + delta);
		cands.Add(mid - delta);
	}

	// Score each candidate: prefer clear + shorter total length
	auto ThreeSeg = [&](double mx) -> Vector<Pointf> {
		// src → (mx, src.y) → (mx, dst.y) → dst
		Vector<Pointf> segs;
		segs.Add(src);
		if (fabs(src.y - dst.y) < 0.5) {
			segs.Add(dst);
		} else {
			segs.Add(Pointf(mx, src.y));
			segs.Add(Pointf(mx, dst.y));
			segs.Add(dst);
		}
		return segs;
	};

	auto SegsBlocked = [&](const Vector<Pointf>& segs) -> bool {
		for (int i = 0; i + 1 < segs.GetCount(); i++)
			if (PathHitsAny(segs[i], segs[i+1], obstacles, MARGIN)) return true;
		return false;
	};

	auto PathLen = [&](const Vector<Pointf>& segs) -> double {
		double d = 0;
		for (int i = 0; i + 1 < segs.GetCount(); i++) d += SegLen(segs[i], segs[i+1]);
		return d;
	};

	Vector<Pointf> best;
	double best_len = 1e18;
	for (double mx : cands) {
		Vector<Pointf> segs = ThreeSeg(mx);
		if (!SegsBlocked(segs)) {
			double l = PathLen(segs);
			if (l < best_len) { best_len = l; best = pick(segs); }
		}
	}
	if (!best.IsEmpty()) return best;

	// All simple 3-seg routes blocked: route around the union bounding box of obstacles
	// that are actually hit by the direct route
	Rectf union_box(src.x, src.y, src.x, src.y);
	bool any = false;
	Pointf direct_mid((src.x+dst.x)/2, (src.y+dst.y)/2);
	for (const Rectf& r : obstacles) {
		if (SegmentHitsObstacle(src, dst, r, MARGIN) ||
		    SegmentHitsObstacle(src, direct_mid, r, MARGIN) ||
		    SegmentHitsObstacle(direct_mid, dst, r, MARGIN)) {
			if (!any) { union_box = r; any = true; }
			else {
				union_box.left   = min(union_box.left,   r.left);
				union_box.top    = min(union_box.top,    r.top);
				union_box.right  = max(union_box.right,  r.right);
				union_box.bottom = max(union_box.bottom, r.bottom);
			}
		}
	}
	if (!any) {
		// No real obstacle intersects — return simple 3-seg
		return ThreeSeg((src.x + dst.x) * 0.5);
	}

	// Route around: go to right of union box, then vertical, then to dst
	// Pick side (right or left) that minimises total travel
	double right_x = union_box.right + MARGIN * 3;
	double left_x  = union_box.left  - MARGIN * 3;
	double dist_r  = fabs(src.x - right_x) + fabs(dst.x - right_x);
	double dist_l  = fabs(src.x - left_x)  + fabs(dst.x - left_x);
	double jog_x   = (dist_r <= dist_l) ? right_x : left_x;

	Vector<Pointf> route;
	route.Add(src);
	route.Add(Pointf(jog_x, src.y));
	route.Add(Pointf(jog_x, dst.y));
	route.Add(dst);
	return route;
}

// ─── Simple ─────────────────────────────────────────────────────────────────

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

// ─── Curved ─────────────────────────────────────────────────────────────────
// Finds the shortest clear path via visibility graph, then rounds the corners
// with bezier arcs.

static RouteResponse RouteCurved(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;

	// If direct line is clear, just use a simple bezier
	if (!PathHitsAny(src, dst, req.obstacles, 6.0)) {
		double hdx = max(abs(dst.x - src.x) * 0.4, 30.0);
		SampleBezier(res.path, src,
		             Pointf(src.x + hdx, src.y),
		             Pointf(dst.x - hdx, dst.y),
		             dst, 24);
		return res;
	}

	// Visibility graph → shortest clear polyline → round corners
	Vector<Pointf> raw = VisibilityShortestPath(src, dst, req.obstacles, 8.0);
	Vector<Pointf> rounded;
	RoundedPolyline(rounded, raw, 18.0);
	res.path = pick(rounded);
	return res;
}

// ─── Schematic ───────────────────────────────────────────────────────────────

static RouteResponse RouteSchematic(const RouteRequest& req)
{
	RouteResponse res;
	Vector<Pointf> ortho = OrthoRoute(req.source_pos, req.target_pos, req.obstacles);
	// Apply small 4-px corner radius for a neat schematic look
	RoundedPolyline(res.path, ortho, 4.0);
	return res;
}

// ─── Realistic (catenary) ────────────────────────────────────────────────────

static RouteResponse RouteRealistic(const RouteRequest& req, double slack_factor)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;
	const int N = 32;
	double dist = SegLen(src, dst);
	double slack = dist * slack_factor;
	double sag   = slack * 0.8;

	double phase = req.anim_phase * 2.0 * M_PI;
	double sway  = sin(phase) * dist * 0.015;

	double len   = dist > 0.001 ? dist : 0.001;
	double perp_x = -(dst.y - src.y) / len;
	double perp_y =  (dst.x - src.x) / len;

	res.path.Reserve(N + 1);
	for (int i = 0; i <= N; i++) {
		double t  = (double)i / N;
		double cx = src.x + (dst.x - src.x) * t;
		double cy = src.y + (dst.y - src.y) * t;
		double drop = sag * 4.0 * t * (1.0 - t);
		double sw   = sway * sin(t * M_PI);
		res.path.Add(Pointf(cx + sw * perp_x,
		                    cy + drop + sw * perp_y));
	}
	return res;
}

// ─── PCB ─────────────────────────────────────────────────────────────────────
// Uses H/V/45° routing (like real PCB autorouters).
// Algorithm:
//   1. Find shortest clear path using visibility graph.
//   2. Snap waypoints to H/V/45° segments.
//   3. Assign layers: straight/horizontal → front (0), diagonal → back (1).
//      When two wires would share the same H/V track, route one on back layer.
//   4. Mark bend points as via_indices for through-hole via rendering.

static double SnapAngle(double angle_deg)
{
	// Snap to nearest multiple of 45°
	return round(angle_deg / 45.0) * 45.0;
}

static Vector<Pointf> HV45Route(Pointf src, Pointf dst, const Vector<Rectf>& obstacles)
{
	// For H/V/45°: prefer going horizontal first to an X midpoint, then diagonally,
	// then horizontally to destination.
	// But use obstacle-aware routing if direct route is blocked.

	// Try direct H/V/45° decomposition:
	// Δx = dx, Δy = dy.  Use 45° diagonal for the min(|dx|,|dy|) portion,
	// then straight for the remainder.
	double dx = dst.x - src.x, dy = dst.y - src.y;
	double adx = fabs(dx), ady = fabs(dy);
	double diag = min(adx, ady);
	double sx = (dx >= 0 ? 1.0 : -1.0), sy = (dy >= 0 ? 1.0 : -1.0);

	Vector<Pointf> direct;
	direct.Add(src);
	if (adx > ady + 0.5) {
		// More horizontal: diagonal first, then horizontal
		Pointf corner1(src.x + sx * diag, src.y + sy * diag);
		direct.Add(corner1);
		direct.Add(dst);
	} else if (ady > adx + 0.5) {
		// More vertical: horizontal first, then diagonal
		Pointf corner1(src.x + sx * (adx > 0.5 ? adx : 0), src.y);
		if (adx > 0.5) {
			direct.Add(corner1);
			direct.Add(dst);
		} else {
			// Pure vertical: use diagonal + horizontal stub
			Pointf corner2(src.x + sy * (ady - adx), src.y + sy * (ady - adx));
			direct.Add(Pointf(src.x, src.y + sy * (ady - adx)));
			direct.Add(dst);
		}
	} else {
		// Equal: pure 45° diagonal
		direct.Add(dst);
	}

	// Check if direct route is clear
	bool blocked = false;
	for (int i = 0; i + 1 < direct.GetCount(); i++)
		if (PathHitsAny(direct[i], direct[i+1], obstacles, 6.0)) { blocked = true; break; }
	if (!blocked) return direct;

	// Blocked: fall back to visibility shortest path then quantize to H/V/45°
	Vector<Pointf> raw = VisibilityShortestPath(src, dst, obstacles, 8.0);
	// Quantize each segment direction to nearest 45°
	Vector<Pointf> quantized;
	quantized.Add(raw[0]);
	Pointf cur = raw[0];
	for (int i = 1; i < raw.GetCount(); i++) {
		double seg_dx = raw[i].x - cur.x;
		double seg_dy = raw[i].y - cur.y;
		double angle  = atan2(seg_dy, seg_dx) * 180.0 / M_PI;
		double snapped = SnapAngle(angle) * M_PI / 180.0;
		double len = SegLen(cur, raw[i]);
		Pointf next(cur.x + len * cos(snapped), cur.y + len * sin(snapped));
		quantized.Add(next);
		cur = next;
	}
	return quantized;
}

static RouteResponse RoutePCB(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;

	Vector<Pointf> pts = HV45Route(src, dst, req.obstacles);

	// Build path and segment layers.
	// H/V segments → layer 0 (front copper, green)
	// 45° diagonal segments → layer 1 (back copper, slightly blue-shifted)
	res.path = pick(pts);

	for (int i = 0; i + 1 < res.path.GetCount(); i++) {
		double sdx = res.path[i+1].x - res.path[i].x;
		double sdy = res.path[i+1].y - res.path[i].y;
		double angle = atan2(fabs(sdy), fabs(sdx)) * 180.0 / M_PI;
		// 0° = horizontal (H), 90° = vertical (V), 45° = diagonal
		bool is_diagonal = (fabs(angle - 45.0) < 5.0);
		res.seg_layer.Add(is_diagonal ? 1 : 0);
	}

	// Via points at every bend (interior vertices)
	for (int i = 1; i + 1 < res.path.GetCount(); i++)
		res.via_indices.Add(i);

	return res;
}

// ─── Dispatch ───────────────────────────────────────────────────────────────

RouteResponse BezierRoutingPolicy::Route(const RouteRequest& req)
{
	switch (req.style) {
	case EdgeStyle::Simple:         return RouteSimple(req);
	case EdgeStyle::Curved:         return RouteCurved(req);
	case EdgeStyle::Schematic:      return RouteSchematic(req);
	case EdgeStyle::RealisticTight: return RouteRealistic(req, 0.18);
	case EdgeStyle::RealisticLoose: return RouteRealistic(req, 0.50);
	case EdgeStyle::PCB:            return RoutePCB(req);
	default:                        return RouteSimple(req);
	}
}

} // namespace Node

} // namespace Upp
