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
	double dx = b.x - a.x, dy = b.y - a.y;
	return sqrt(dx*dx + dy*dy);
}

// ─── Visibility-graph shortest path ─────────────────────────────────────────

static Vector<Pointf> VisibilityShortestPath(
    Pointf src, Pointf dst,
    const Vector<Rectf>& obstacles, double margin = 8.0)
{
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
	Vector<double> dist(n, 1e18);
	Vector<int>    prev(n, -1);
	dist[0] = 0;
	Vector<bool> visited(n, false);
	for (int iter = 0; iter < n; iter++) {
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

	Vector<Pointf> path;
	if (dist[1] >= 1e17) { path.Add(src); path.Add(dst); return path; }
	Vector<int> idx;
	for (int cur = 1; cur >= 0; ) {
		idx.Add(cur);
		int p = prev[cur];
		if (p < 0 || p == cur) break;
		cur = p;
	}
	for (int i = idx.GetCount() - 1; i >= 0; i--)
		path.Add(pts[idx[i]]);
	return path;
}

// Round a polyline: replace each interior vertex with a quadratic bezier arc.
static void RoundedPolyline(Vector<Pointf>& out, const Vector<Pointf>& pts, double radius)
{
	if (pts.GetCount() < 2) return;
	out.Add(pts[0]);
	for (int i = 1; i < pts.GetCount() - 1; i++) {
		Pointf prev = pts[i-1], cur = pts[i], next = pts[i+1];
		double d1 = SegLen(prev, cur), d2 = SegLen(cur, next);
		double r = min(radius, min(d1, d2) * 0.4);
		if (r < 1.0) { out.Add(cur); continue; }
		double t1 = r / d1, t2 = r / d2;
		Pointf e((1-t1)*prev.x + t1*cur.x, (1-t1)*prev.y + t1*cur.y);
		Pointf x(cur.x + t2*(next.x-cur.x), cur.y + t2*(next.y-cur.y));
		const int ARC = 6;
		for (int j = 0; j <= ARC; j++) {
			double t = (double)j / ARC;
			out.Add(Pointf((1-t)*(1-t)*e.x + 2*(1-t)*t*cur.x + t*t*x.x,
			               (1-t)*(1-t)*e.y + 2*(1-t)*t*cur.y + t*t*x.y));
		}
	}
	out.Add(pts[pts.GetCount() - 1]);
}

// ─── Orthogonal channel router for Schematic ─────────────────────────────────

static Vector<Pointf> OrthoRoute(Pointf src, Pointf dst,
                                 const Vector<Rectf>& obstacles)
{
	const double MARGIN = 6.0;
	const double STEP   = 20.0;

	auto ThreeSeg = [&](double mx) -> Vector<Pointf> {
		Vector<Pointf> segs;
		segs.Add(src);
		if (fabs(src.y - dst.y) < 0.5) { segs.Add(dst); return segs; }
		segs.Add(Pointf(mx, src.y));
		segs.Add(Pointf(mx, dst.y));
		segs.Add(dst);
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

	Vector<double> cands;
	cands.Add((src.x + dst.x) * 0.5);
	for (const Rectf& r : obstacles) {
		cands.Add(r.left  - MARGIN * 2);
		cands.Add(r.right + MARGIN * 2);
	}
	double mid = (src.x + dst.x) * 0.5;
	for (double delta = 0; delta < 800; delta += STEP) {
		cands.Add(mid + delta);
		cands.Add(mid - delta);
	}

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

	// Fall back: route around union box of blocking obstacles
	Rectf union_box; bool any = false;
	for (const Rectf& r : obstacles) {
		if (SegsBlocked(ThreeSeg((src.x+dst.x)*0.5)) ||
		    SegmentHitsObstacle(src, dst, r, MARGIN)) {
			if (!any) { union_box = r; any = true; }
			else {
				union_box.left   = min(union_box.left,   r.left);
				union_box.top    = min(union_box.top,    r.top);
				union_box.right  = max(union_box.right,  r.right);
				union_box.bottom = max(union_box.bottom, r.bottom);
			}
		}
	}
	if (!any) return ThreeSeg(mid);

	double right_x = union_box.right + MARGIN * 3;
	double left_x  = union_box.left  - MARGIN * 3;
	double jog_x = (fabs(src.x - right_x) + fabs(dst.x - right_x) <=
	                fabs(src.x - left_x)  + fabs(dst.x - left_x))
	               ? right_x : left_x;
	Vector<Pointf> route;
	route.Add(src); route.Add(Pointf(jog_x, src.y));
	route.Add(Pointf(jog_x, dst.y)); route.Add(dst);
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

static RouteResponse RouteCurved(const RouteRequest& req)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;
	if (!PathHitsAny(src, dst, req.obstacles, 6.0)) {
		double hdx = max(abs(dst.x - src.x) * 0.4, 30.0);
		SampleBezier(res.path, src,
		             Pointf(src.x + hdx, src.y),
		             Pointf(dst.x - hdx, dst.y),
		             dst, 24);
		return res;
	}
	Vector<Pointf> raw = VisibilityShortestPath(src, dst, req.obstacles, 8.0);
	RoundedPolyline(res.path, raw, 18.0);
	return res;
}

// ─── Schematic ───────────────────────────────────────────────────────────────

static RouteResponse RouteSchematic(const RouteRequest& req)
{
	RouteResponse res;
	Vector<Pointf> ortho = OrthoRoute(req.source_pos, req.target_pos, req.obstacles);
	RoundedPolyline(res.path, ortho, 4.0);
	return res;
}

// ─── Realistic (catenary) ────────────────────────────────────────────────────

static RouteResponse RouteRealistic(const RouteRequest& req, double slack_factor)
{
	RouteResponse res;
	Pointf src = req.source_pos, dst = req.target_pos;
	const int N = 32;
	double dist  = SegLen(src, dst);
	double sag   = dist * slack_factor * 0.8;
	double phase = req.anim_phase * 2.0 * M_PI;
	double sway  = sin(phase) * dist * 0.015;
	double len   = dist > 0.001 ? dist : 0.001;
	double perp_x = -(dst.y - src.y) / len;
	double perp_y =  (dst.x - src.x) / len;
	res.path.Reserve(N + 1);
	for (int i = 0; i <= N; i++) {
		double t   = (double)i / N;
		double cx  = src.x + (dst.x - src.x) * t;
		double cy  = src.y + (dst.y - src.y) * t;
		double drop = sag * 4.0 * t * (1.0 - t);
		double sw   = sway * sin(t * M_PI);
		res.path.Add(Pointf(cx + sw * perp_x, cy + drop + sw * perp_y));
	}
	return res;
}

// ─── PCB grid / Lee maze router ──────────────────────────────────────────────

// Cell states
enum : int8_t {
	CELL_FREE    = 0,
	CELL_BLOCKED = 1,   // node box
	CELL_TRACE0  = 2,   // front-copper trace
	CELL_TRACE1  = 3,   // back-copper trace
};

// A 2-layer grid.  Layer index 0 = front, 1 = back.
// A cell is "available on layer L" when it is either FREE or holds the *other* layer's trace.
struct PcbGrid {
	int    gw = 0, gh = 0;       // grid dimensions in cells
	double ox = 0, oy = 0;       // world origin of cell (0,0)
	double cell = 10.0;          // world units per cell
	Vector<int8_t> layer[2];     // per-cell state, one array per layer

	void Init(const Rectf& bounds, double cell_size, const Vector<Rectf>& node_boxes)
	{
		cell = cell_size;
		const double PAD = cell * 3;   // padding around scene bounds
		ox = floor((bounds.left   - PAD) / cell) * cell;
		oy = floor((bounds.top    - PAD) / cell) * cell;
		double x1 = ceil((bounds.right  + PAD) / cell) * cell;
		double y1 = ceil((bounds.bottom + PAD) / cell) * cell;
		gw = max(1, (int)((x1 - ox) / cell) + 1);
		gh = max(1, (int)((y1 - oy) / cell) + 1);
		// Clamp to a reasonable maximum (e.g. 400×400 = 160k cells)
		gw = min(gw, 400);
		gh = min(gh, 400);
		for (int L = 0; L < 2; L++) {
			layer[L].SetCount(gw * gh, CELL_FREE);
		}
		// Mark node boxes as blocked on both layers
		const double NODE_PAD = cell * 0.5; // half-cell margin around nodes
		for (const Rectf& r : node_boxes) {
			int x0c = max(0, (int)floor((r.left  - NODE_PAD - ox) / cell));
			int y0c = max(0, (int)floor((r.top   - NODE_PAD - oy) / cell));
			int x1c = min(gw-1, (int)ceil((r.right  + NODE_PAD - ox) / cell));
			int y1c = min(gh-1, (int)ceil((r.bottom + NODE_PAD - oy) / cell));
			for (int cy = y0c; cy <= y1c; cy++)
				for (int cx = x0c; cx <= x1c; cx++) {
					layer[0][cy * gw + cx] = CELL_BLOCKED;
					layer[1][cy * gw + cx] = CELL_BLOCKED;
				}
		}
	}

	// Convert world point to nearest grid cell, clamped to grid.
	Point WorldToCell(Pointf p) const {
		return Point(
		    max(0, min(gw-1, (int)round((p.x - ox) / cell))),
		    max(0, min(gh-1, (int)round((p.y - oy) / cell))));
	}

	// Convert cell to world point (cell centre).
	Pointf CellToWorld(int cx, int cy) const {
		return Pointf(ox + cx * cell, oy + cy * cell);
	}

	bool InBounds(int cx, int cy) const {
		return cx >= 0 && cx < gw && cy >= 0 && cy < gh;
	}

	int8_t Get(int L, int cx, int cy) const { return layer[L][cy * gw + cx]; }
	void   Set(int L, int cx, int cy, int8_t v) { layer[L][cy * gw + cx] = v; }
};

// Lee BFS on one layer.  Returns cell path src→dst, or empty if unreachable.
// allow_diagonal: if true, 8-connected; if false, 4-connected (pure H/V).
static Vector<Point> LeeBFS(PcbGrid& grid, int L,
                             Point src, Point dst,
                             bool allow_diagonal)
{
	// Guard: if src or dst is blocked, find nearest free cell
	auto NearestFree = [&](Point p) -> Point {
		if (grid.InBounds(p.x, p.y) && grid.Get(L, p.x, p.y) != CELL_BLOCKED)
			return p;
		// Spiral search outward
		for (int r = 1; r < 8; r++)
			for (int dy = -r; dy <= r; dy++)
				for (int dx = -r; dx <= r; dx++) {
					Point q(p.x + dx, p.y + dy);
					if (grid.InBounds(q.x, q.y) && grid.Get(L, q.x, q.y) != CELL_BLOCKED)
						return q;
				}
		return p;
	};
	src = NearestFree(src);
	dst = NearestFree(dst);

	if (src == dst) {
		Vector<Point> p; p.Add(src); return p;
	}

	// Weighted pathfinding: prev[] for path reconstruction, dist_arr[] for costs.
	int N = grid.gw * grid.gh;
	Vector<int> prev(N, -1);
	int src_idx = src.y * grid.gw + src.x;
	int dst_idx = dst.y * grid.gw + dst.x;
	prev[src_idx] = src_idx; // mark source

	// Directions: H/V first, diagonals second
	static const int DX4[] = { 1,-1, 0, 0 };
	static const int DY4[] = { 0, 0, 1,-1 };
	static const int DX8[] = { 1,-1, 0, 0, 1,-1, 1,-1 };
	static const int DY8[] = { 0, 0, 1,-1, 1,-1,-1, 1 };
	const int* DX = allow_diagonal ? DX8 : DX4;
	const int* DY = allow_diagonal ? DY8 : DY4;
	int NDIRS    = allow_diagonal ? 8 : 4;

	// Weighted BFS: free cell costs 1, existing trace costs TRACE_COST.
	// High penalty steers routes around already-routed traces.
	// Uses a circular bucket queue (dial's algorithm) — O(N * max_cost).
	const int TRACE_COST = 30;
	const int MAX_COST   = TRACE_COST + 1;
	Vector<int> dist_arr(N, INT_MAX);
	dist_arr[src_idx] = 0;
	// Bucket queue: buckets[cost % MAX_COST] holds cell indices at that distance
	Vector<Vector<int>> buckets(MAX_COST);
	buckets[0].Add(src_idx);
	int cur_cost = 0;
	int remaining = 1;

	while (remaining > 0) {
		// Advance to next non-empty bucket
		while (buckets[cur_cost % MAX_COST].IsEmpty()) {
			cur_cost++;
			if (cur_cost > N * MAX_COST) break; // safety
		}
		if (cur_cost > N * MAX_COST) break;

		int bucket_idx = cur_cost % MAX_COST;
		int cur = buckets[bucket_idx].Pop();
		remaining--;

		if (dist_arr[cur] != cur_cost) continue; // stale entry
		if (cur == dst_idx) break;

		int cx = cur % grid.gw, cy = cur / grid.gw;
		for (int d = 0; d < NDIRS; d++) {
			int nx = cx + DX[d], ny = cy + DY[d];
			if (!grid.InBounds(nx, ny)) continue;
			int nidx = ny * grid.gw + nx;
			int8_t cell = grid.Get(L, nx, ny);
			if (cell == CELL_BLOCKED) continue;
			int step_cost = (cell == CELL_FREE) ? 1 : TRACE_COST;
			int new_dist = cur_cost + step_cost;
			if (new_dist < dist_arr[nidx]) {
				dist_arr[nidx] = new_dist;
				prev[nidx] = cur;
				buckets[new_dist % MAX_COST].Add(nidx);
				remaining++;
			}
		}
	}

	// Reconstruct
	Vector<Point> path;
	if (prev[dst_idx] < 0) return path; // unreachable
	for (int cur = dst_idx; cur != src_idx; ) {
		path.Add(Point(cur % grid.gw, cur / grid.gw));
		int p = prev[cur];
		if (p == cur) break;
		cur = p;
	}
	path.Add(src);
	// Reverse
	for (int i = 0, j = path.GetCount()-1; i < j; i++, j--)
		Swap(path[i], path[j]);
	return path;
}

// Compress collinear/diagonal runs: keep only bend points + endpoints.
static Vector<Point> CompressCellPath(const Vector<Point>& raw)
{
	Vector<Point> out;
	if (raw.GetCount() < 2) { out <<= raw; return out; }
	out.Add(raw[0]);
	for (int i = 1; i < raw.GetCount() - 1; i++) {
		int dx1 = raw[i].x - raw[i-1].x, dy1 = raw[i].y - raw[i-1].y;
		int dx2 = raw[i+1].x - raw[i].x, dy2 = raw[i+1].y - raw[i].y;
		if (dx1 != dx2 || dy1 != dy2) out.Add(raw[i]); // direction changed
	}
	out.Add(raw[raw.GetCount()-1]);
	return out;
}

// Mark grid cells occupied by a routed cell path on layer L.
static void MarkPath(PcbGrid& grid, int L, const Vector<Point>& path)
{
	int8_t val = (L == 0) ? CELL_TRACE0 : CELL_TRACE1;
	// Walk each segment cell-by-cell using Bresenham
	for (int i = 0; i + 1 < path.GetCount(); i++) {
		int x0 = path[i].x, y0 = path[i].y;
		int x1 = path[i+1].x, y1 = path[i+1].y;
		int dx = abs(x1-x0), dy = abs(y1-y0);
		int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
		int err = dx - dy;
		while (true) {
			if (grid.InBounds(x0, y0) && grid.Get(0, x0, y0) != CELL_BLOCKED)
				grid.Set(L, x0, y0, val);
			if (x0 == x1 && y0 == y1) break;
			int e2 = 2 * err;
			if (e2 > -dy) { err -= dy; x0 += sx; }
			if (e2 <  dx) { err += dx; y0 += sy; }
		}
	}
}

// Route one net via Lee BFS.  Returns (cell path, layer used).
// Temporarily unblocks a 1-cell radius around src/dst endpoints so BFS can
// enter/exit through the node-boundary margin that Init() marks as blocked.
static Vector<Point> RoutePCBNet(PcbGrid& grid, Point src, Point dst,
                                 bool allow_diagonal, int& layer_used)
{
	// Save and unblock BLOCKED cells in a 1-cell radius around each endpoint
	struct SavedCell : Moveable<SavedCell> { Point p; int L; int8_t val; };
	Vector<SavedCell> saved;
	const int R = 1;
	auto Unblock = [&](Point center) {
		for (int dy = -R; dy <= R; dy++)
			for (int dx = -R; dx <= R; dx++) {
				Point q(center.x + dx, center.y + dy);
				if (!grid.InBounds(q.x, q.y)) continue;
				for (int L = 0; L < 2; L++) {
					int8_t v = grid.Get(L, q.x, q.y);
					if (v == CELL_BLOCKED) {
						SavedCell sc; sc.p = q; sc.L = L; sc.val = v;
						saved.Add(pick(sc));
						grid.Set(L, q.x, q.y, CELL_FREE);
					}
				}
			}
	};
	auto Restore = [&]() {
		for (const SavedCell& sc : saved)
			grid.Set(sc.L, sc.p.x, sc.p.y, sc.val);
	};

	Unblock(src);
	Unblock(dst);

	// Try layer 0 first, then layer 1
	for (int L = 0; L < 2; L++) {
		Vector<Point> path = LeeBFS(grid, L, src, dst, allow_diagonal);
		if (!path.IsEmpty()) {
			layer_used = L;
			Vector<Point> compressed = CompressCellPath(path);
			Restore();
			MarkPath(grid, L, path);
			return compressed;
		}
	}

	Restore();
	layer_used = 0;
	Vector<Point> fallback;
	fallback.Add(src); fallback.Add(dst);
	return fallback;
}

// Convert cell path to world-space RouteResponse.
static RouteResponse CellPathToResponse(const Vector<Point>& cpath, int layer_used,
                                        const PcbGrid& grid, bool allow_diagonal)
{
	RouteResponse res;
	if (cpath.IsEmpty()) return res;

	// Build world points
	for (const Point& cp : cpath)
		res.path.Add(grid.CellToWorld(cp.x, cp.y));

	// All segments are on layer_used
	for (int i = 0; i + 1 < res.path.GetCount(); i++)
		res.seg_layer.Add(layer_used);

	// Via dots only at layer transitions (none on a single-layer route)

	return res;
}

// ─── PCB variants ────────────────────────────────────────────────────────────

// HV-Fast: greedy obstacle-avoidance (visibility graph) then snap to H/V grid segments.
// Marks grid but uses non-BFS routing (fast, ~O(n) per net).
static RouteResponse RoutePCBHVFast(const RouteRequest& req, PcbGrid& grid)
{
	Pointf src = req.source_pos, dst = req.target_pos;
	Point csrc = grid.WorldToCell(src);
	Point cdst = grid.WorldToCell(dst);

	// Check if H/V 3-segment route is free on layer 0.
	// Endpoint cells (csrc/cdst) are inside node margin — skip them in blocked check.
	auto TryCellRoute = [&](int mx, int L) -> Vector<Point> {
		// src → (mx, csrc.y) → (mx, cdst.y) → dst  (all H/V)
		Vector<Point> pts;
		pts.Add(csrc);
		if (csrc.y != cdst.y) {
			pts.Add(Point(mx, csrc.y));
			pts.Add(Point(mx, cdst.y));
		}
		pts.Add(cdst);
		// Check no BLOCKED cell along each segment, skipping endpoints
		for (int i = 0; i + 1 < pts.GetCount(); i++) {
			int x0 = pts[i].x, y0 = pts[i].y;
			int x1 = pts[i+1].x, y1 = pts[i+1].y;
			int steps = max(abs(x1-x0), abs(y1-y0));
			int s_start = (i == 0) ? 1 : 0;                        // skip csrc
			int s_end   = (i == pts.GetCount()-2) ? steps-1 : steps; // skip cdst
			for (int s = s_start; s <= s_end; s++) {
				int cx = (steps > 0) ? x0 + (x1-x0)*s/steps : x0;
				int cy = (steps > 0) ? y0 + (y1-y0)*s/steps : y0;
				if (!grid.InBounds(cx, cy)) continue;
				int8_t c = grid.Get(L, cx, cy);
				if (c == CELL_BLOCKED || c == CELL_TRACE0 || c == CELL_TRACE1)
					return Vector<Point>();
			}
		}
		return pts;
	};

	// Try mid-X candidates on both layers
	Vector<int> cands_x;
	cands_x.Add((csrc.x + cdst.x) / 2);
	for (const Rectf& r : req.obstacles) {
		cands_x.Add(grid.WorldToCell(Pointf(r.left  - 15, 0)).x);
		cands_x.Add(grid.WorldToCell(Pointf(r.right + 15, 0)).x);
	}
	for (int delta = 0; delta <= 30; delta += 2) {
		cands_x.Add((csrc.x + cdst.x)/2 + delta);
		cands_x.Add((csrc.x + cdst.x)/2 - delta);
	}

	for (int L = 0; L < 2; L++) {
		for (int mx : cands_x) {
			mx = max(0, min(grid.gw-1, mx));
			Vector<Point> pts = TryCellRoute(mx, L);
			if (!pts.IsEmpty()) {
				MarkPath(grid, L, pts);
				return CellPathToResponse(CompressCellPath(pts), L, grid, false);
			}
		}
	}

	// Full BFS fallback
	int layer_used = 0;
	Vector<Point> bfs = RoutePCBNet(grid, csrc, cdst, false, layer_used);
	return CellPathToResponse(bfs, layer_used, grid, false);
}

// HV-Lee: full Lee BFS, pure H/V (4-connected).
static RouteResponse RoutePCBHVLee(const RouteRequest& req, PcbGrid& grid)
{
	Point csrc = grid.WorldToCell(req.source_pos);
	Point cdst = grid.WorldToCell(req.target_pos);
	int layer_used = 0;
	Vector<Point> path = RoutePCBNet(grid, csrc, cdst, false, layer_used);
	return CellPathToResponse(path, layer_used, grid, false);
}

// PCB45: Lee/BFS with H/V/45° moves (8-connected).
static RouteResponse RoutePCB45(const RouteRequest& req, PcbGrid& grid)
{
	Point csrc = grid.WorldToCell(req.source_pos);
	Point cdst = grid.WorldToCell(req.target_pos);
	int layer_used = 0;
	Vector<Point> path = RoutePCBNet(grid, csrc, cdst, true, layer_used);
	return CellPathToResponse(path, layer_used, grid, true);
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
	case EdgeStyle::PCBHVFast:      return RoutePCBHVFast(req, *pcb_grid);
	case EdgeStyle::PCBHVLee:       return RoutePCBHVLee(req, *pcb_grid);
	case EdgeStyle::PCB45:          return RoutePCB45(req, *pcb_grid);
	default:                        return RouteSimple(req);
	}
}

void BezierRoutingPolicy::BeginBatch(const Rectf& scene_bounds,
                                     const Vector<Rectf>& node_boxes,
                                     EdgeStyle style)
{
	bool is_pcb = (style == EdgeStyle::PCBHVFast || style == EdgeStyle::PCBHVLee ||
	               style == EdgeStyle::PCB45);
	if (!is_pcb) { pcb_grid.Clear(); return; }

	pcb_grid = MakeOne<PcbGrid>();
	pcb_grid->Init(scene_bounds, 10.0, node_boxes);
}

} // namespace Node

} // namespace Upp
