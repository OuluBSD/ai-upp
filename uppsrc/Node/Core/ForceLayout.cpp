#include "ForceLayout.h"

namespace Upp {

namespace Node {

// ---------------------------------------------------------------------------
// ForceRefine — two-phase rigid-box layout
//
// All collision tests use Rect::Intersects — no hand-rolled half-extents math.
//
// Phase 1 (iters*6/10): compact each group — attract nodes to group centroid,
//   resolve node-node overlaps.
// Phase 2 (iters*4/10): separate groups — attract groups to global centroid
//   (rigid-body), resolve group-group overlaps (moves all nodes in group),
//   re-settle nodes after group motion.
//
// node gap  = NODE_PAD * cell_size  (added to each side of each node's rect)
// group gap = GROUP_PAD * cell_size (added to each side of each group's bbox)
// ---------------------------------------------------------------------------

static const double NODE_PAD    = 3.0;
static const double GROUP_PAD   = 3.0;
static const int    MAX_NODE_IT = 400;
static const int    MAX_GRP_IT  = 1000;

static bool s_force_verbose = false;
void SetForceVerbose(bool v) { s_force_verbose = v; }
bool IsForceVerbose()        { return s_force_verbose; }

// Format helper — U++ Format doesn't support %.Xf
static String FI(double v) { return IntStr((int)round(v)); }

// Verbose output — writes directly to fd 2 (stderr) so it appears in the
// terminal even when the process is a GUI app (U++ Cout/Cerr may be redirected).
static void VOut(const String& s)
{
	fputs(s, stderr);
	fflush(stderr);
}

// Build Rect from (x0,y0,x1,y1) — positive-y-down coordinate system
static Rect ToRect(double x0, double y0, double x1, double y1)
{
	// Rect requires left<=right, top<=bottom (y grows downward in U++ GUI)
	return Rect((int)floor(x0), (int)floor(y0), (int)ceil(x1), (int)ceil(y1));
}

static void RebuildGroup(int g, int N,
                         const Vector<int>&    gid,
                         const Vector<double>& cx, const Vector<double>& cy,
                         const Vector<double>& hw, const Vector<double>& hh,
                         double& x0, double& y0, double& x1, double& y1)
{
	x0 =  1e18; y0 =  1e18; x1 = -1e18; y1 = -1e18;
	for(int i = 0; i < N; i++) {
		if(gid[i] != g) continue;
		x0 = min(x0, cx[i]-hw[i]); y0 = min(y0, cy[i]-hh[i]);
		x1 = max(x1, cx[i]+hw[i]); y1 = max(y1, cy[i]+hh[i]);
	}
}

void ForceRefine(Vector<ForceItem>& items,
                 int                num_groups,
                 double             cell_size,
                 int                iters)
{
	bool verbose = s_force_verbose;
	int N = items.GetCount();
	if(N < 2) return;

	int NG = num_groups;
	double node_pad = NODE_PAD  * cell_size;
	double grp_pad  = GROUP_PAD * cell_size;

	// Node centres + half-extents
	Vector<double> cx(N), cy(N), hw(N), hh(N);
	for(int i = 0; i < N; i++) {
		hw[i] = items[i].sz.cx * 0.5;
		hh[i] = items[i].sz.cy * 0.5;
		cx[i] = items[i].pos.x + hw[i];
		cy[i] = items[i].pos.y + hh[i];
	}

	// Per-node port-based extra padding
	Vector<double> pp(N);
	for(int i = 0; i < N; i++)
		pp[i] = 0.25 * items[i].port_count * cell_size;

	// Group membership
	Vector<int> gid(N);
	for(int i = 0; i < N; i++) gid[i] = items[i].group_id;

	Vector<int> gcn(max(NG,1), 0);
	for(int i = 0; i < N; i++)
		if(gid[i] >= 0 && gid[i] < NG) gcn[gid[i]]++;

	// Group bounding boxes (x0,y0,x1,y1)
	Vector<double> gbx0(max(NG,1)), gby0(max(NG,1)),
	               gbx1(max(NG,1)), gby1(max(NG,1));

	auto RebuildAll = [&]() {
		for(int g = 0; g < NG; g++)
			if(gcn[g] > 0)
				RebuildGroup(g, N, gid, cx, cy, hw, hh,
				             gbx0[g], gby0[g], gbx1[g], gby1[g]);
	};

	// Node rect with padding (positive-y-down)
	auto NodeRect = [&](int i, double pad) -> Rect {
		double p = node_pad + pp[i] + pad;
		return ToRect(cx[i]-hw[i]-p, cy[i]-hh[i]-p,
		              cx[i]+hw[i]+p, cy[i]+hh[i]+p);
	};

	// Group rect with padding
	auto GroupRect = [&](int g, double pad) -> Rect {
		double p = grp_pad + pad;
		return ToRect(gbx0[g]-p, gby0[g]-p, gbx1[g]+p, gby1[g]+p);
	};

	// Count node-node overlaps using Rect::Intersects
	auto CountNodeOv = [&]() -> int {
		int cnt = 0;
		for(int i = 0; i < N; i++)
			for(int j = i+1; j < N; j++)
				if(NodeRect(i,0).Intersects(NodeRect(j,0))) cnt++;
		return cnt;
	};

	// Count group-group overlaps using Rect::Intersects
	auto CountGroupOv = [&]() -> int {
		int cnt = 0;
		for(int ga = 0; ga < NG; ga++) { if(!gcn[ga]) continue;
			for(int gb = ga+1; gb < NG; gb++) { if(!gcn[gb]) continue;
				if(GroupRect(ga,0).Intersects(GroupRect(gb,0))) cnt++;
			}
		}
		return cnt;
	};

	// Print state (verbose) — writes to stderr so it's visible in GUI+terminal
	auto PrintState = [&](const char* phase, int iter) {
		RebuildAll();
		int nov = CountNodeOv();
		int gov = CountGroupOv();
		String s;
		s << phase << "[" << iter << "] node_ov=" << nov << " grp_ov=" << gov << "\n";
		for(int g = 0; g < NG; g++) if(gcn[g] > 0) {
			Rect r = GroupRect(g, 0);
			s << "  grp[" << g << "]=(" << r.left << "," << r.top << ")-("
			  << r.right << "," << r.bottom << ") "
			  << r.Width() << "x" << r.Height() << "\n";
		}
		for(int i = 0; i < N; i++) if(gid[i] < 0) {
			Rect r = NodeRect(i, 0);
			s << "  node[" << i << "]=(" << r.left << "," << r.top << ")-("
			  << r.right << "," << r.bottom << ") "
			  << r.Width() << "x" << r.Height() << "\n";
		}
		for(int ga = 0; ga < NG; ga++) { if(!gcn[ga]) continue;
			for(int gb = ga+1; gb < NG; gb++) { if(!gcn[gb]) continue;
				Rect ra = GroupRect(ga,0), rb = GroupRect(gb,0);
				if(ra.Intersects(rb)) {
					Rect isect = ra & rb;
					s << "  OVERLAP grp[" << ga << "] vs grp[" << gb
					  << "] isect=(" << isect.left << "," << isect.top << ")-("
					  << isect.right << "," << isect.bottom << ") "
					  << isect.Width() << "x" << isect.Height() << "\n";
				}
			}
		}
		VOut(s);
	};

	// Resolve node-node: push apart along min-penetration axis
	auto ResolveNodes = [&]() {
		for(int sep = 0; sep < MAX_NODE_IT; sep++) {
			bool any = false;
			for(int i = 0; i < N; i++) for(int j = i+1; j < N; j++) {
				Rect ri = NodeRect(i,0), rj = NodeRect(j,0);
				if(!ri.Intersects(rj)) continue;
				Rect isect = ri & rj;
				double dx = cx[i]-cx[j], dy = cy[i]-cy[j];
				if(isect.Width() <= isect.Height()) {
					double half = isect.Width() * 0.5 + 0.5;
					double sign = (dx >= 0) ? 1.0 : -1.0;
					cx[i] += half*sign; cx[j] -= half*sign;
				} else {
					double half = isect.Height() * 0.5 + 0.5;
					double sign = (dy >= 0) ? 1.0 : -1.0;
					cy[i] += half*sign; cy[j] -= half*sign;
				}
				any = true;
			}
			if(!any) break;
		}
	};

	// Resolve group-group: move all nodes in each group rigidly.
	// After each fix, restart the sweep from the beginning — a fix can create
	// new overlaps with previously-clean pairs.
	auto ResolveGroups = [&]() {
		if(NG < 2) return;
		for(int sweep = 0; sweep < MAX_GRP_IT; sweep++) {
			bool found = false;
			for(int ga = 0; ga < NG && !found; ga++) { if(!gcn[ga]) continue;
				for(int gb = ga+1; gb < NG && !found; gb++) { if(!gcn[gb]) continue;
					Rect ra = GroupRect(ga,0), rb = GroupRect(gb,0);
					if(!ra.Intersects(rb)) continue;
					Rect isect = ra & rb;
					double ddx = (gbx0[ga]+gbx1[ga])*0.5 - (gbx0[gb]+gbx1[gb])*0.5;
					double ddy = (gby0[ga]+gby1[ga])*0.5 - (gby0[gb]+gby1[gb])*0.5;
					bool horiz = (isect.Width() <= isect.Height());
					double delta = (horiz ? isect.Width() : isect.Height()) * 0.5 + 0.5;
					double sign  = horiz ? ((ddx>=0)?1.:-1.) : ((ddy>=0)?1.:-1.);
					for(int i = 0; i < N; i++) {
						if(gid[i]==ga) { if(horiz) cx[i]+=delta*sign; else cy[i]+=delta*sign; }
						if(gid[i]==gb) { if(horiz) cx[i]-=delta*sign; else cy[i]-=delta*sign; }
					}
					RebuildGroup(ga,N,gid,cx,cy,hw,hh,gbx0[ga],gby0[ga],gbx1[ga],gby1[ga]);
					RebuildGroup(gb,N,gid,cx,cy,hw,hh,gbx0[gb],gby0[gb],gbx1[gb],gby1[gb]);
					found = true;  // restart sweep from (0,1)
				}
			}
			if(!found) return;  // clean pass — done
		}
		RLOG("ForceRefine: WARNING group separation did not converge");
		if(verbose) VOut("WARNING: group separation did not converge!\n");
	};

	int phase1 = iters * 6 / 10;
	int phase2 = iters - phase1;

	if(verbose) {
		RebuildAll();
		VOut(String("ForceRefine start: N=") << N << " NG=" << NG
		     << " cell=" << FI(cell_size) << " node_pad=" << FI(node_pad)
		     << " grp_pad=" << FI(grp_pad)
		     << " phase1=" << phase1 << " phase2=" << phase2 << "\n");
		PrintState("init", 0);
	}

	// ── Phase 1: compact nodes within each group ─────────────────────────────
	// Pull nodes toward group centroid to pack them tightly, then resolve collisions.
	// Skip if initial layout already has no overlaps (e.g. ScriptedLayout prescribed coords).
	{
		RebuildAll();
		int init_ov = CountNodeOv();
		RLOG("ForceRefine P1 initial node_ov=" << init_ov);
		if(init_ov == 0) {
			RLOG("ForceRefine P1 skipped (no initial overlaps)");
		} else {
			for(int iter = 0; iter < phase1; iter++) {
				// Per-group centroid
				Vector<double> gcx(max(NG,1),0.), gcy(max(NG,1),0.);
				for(int i = 0; i < N; i++)
					if(gid[i]>=0 && gid[i]<NG) { gcx[gid[i]]+=cx[i]; gcy[gid[i]]+=cy[i]; }
				for(int g = 0; g < NG; g++)
					if(gcn[g]>0) { gcx[g]/=gcn[g]; gcy[g]/=gcn[g]; }

				// Pull every node 50% toward its group centroid, then resolve collisions.
				double total_move = 0;
				for(int i = 0; i < N; i++) {
					int g = gid[i];
					if(g < 0 || g >= NG || gcn[g] < 2) continue;
					double dx = gcx[g] - cx[i];
					double dy = gcy[g] - cy[i];
					cx[i] += dx * 0.5;
					cy[i] += dy * 0.5;
					total_move += fabs(dx) + fabs(dy);
				}
				ResolveNodes();
				RebuildAll();
				{
					int nov=CountNodeOv(), gov=CountGroupOv();
					RLOG("ForceRefine P1[" << iter << "] node_ov=" << nov << " grp_ov=" << gov << " move=" << (int)total_move);
					if(verbose) PrintState("P1", iter);
					// Converged: no node overlaps remain
					if(nov == 0) {
						RLOG("ForceRefine P1 converged at iter=" << iter);
						break;
					}
				}
			}
		}
	}

	// ── Phase 2: separate groups ──────────────────────────────────────────────
	RebuildAll();
	double scene_cx=0, scene_cy=0; int sn=0;
	for(int g=0;g<NG;g++) if(gcn[g]>0) {
		scene_cx += (gbx0[g]+gbx1[g])*0.5;
		scene_cy += (gby0[g]+gby1[g])*0.5;
		sn++;
	}
	if(sn>0) { scene_cx/=sn; scene_cy/=sn; }

	for(int iter = 0; iter < phase2; iter++) {
		// Pure collision resolution — no attraction (attraction fights separation)
		RebuildAll();
		ResolveGroups();
		ResolveNodes();
		RebuildAll();
		{
			int nov=CountNodeOv(), gov=CountGroupOv();
			RLOG("ForceRefine P2[" << iter << "] node_ov=" << nov << " grp_ov=" << gov);
			if(verbose) PrintState("P2", iter);
			if(nov==0 && gov==0) { RLOG("ForceRefine converged at P2 iter=" << iter); break; }
		}
	}

	// Write back positions + estimated sizes
	for(int i = 0; i < N; i++) {
		items[i].pos.x = cx[i] - hw[i];
		items[i].pos.y = cy[i] - hh[i];
	}
}

// ---------------------------------------------------------------------------
// Scene layout constants (must match Scene.cpp)
// ---------------------------------------------------------------------------
static const double FL_NODE_W     = 200.0;
static const double FL_TITLE_H    = 26.0;
static const double FL_PIN_ROW_H  = 22.0;
static const double FL_SLOT_ROW_H = 24.0;
static const double FL_DOCEDIT_H  = 6 * FL_SLOT_ROW_H;
static const double FL_IMAGE_H    = 140.0;

static Sizef FLEstimateNodeSize(const NodeDoc& n)
{
	double w = n.sz.cx > 1 ? n.sz.cx : FL_NODE_W;
	int in_c = 0, out_c = 0;
	for(const PinDoc& p : n.pins)
		(p.kind == PinKind::Output ? out_c : in_c)++;
	int pin_rows = max(in_c, out_c);
	double slot_h = 0;
	for(const WidgetSlotDoc& s : n.slots) {
		if(s.type == "Image" || s.type == "IMAGE") slot_h += FL_IMAGE_H;
		else if(s.type == "DocEdit")               slot_h += FL_DOCEDIT_H;
		else                                        slot_h += FL_SLOT_ROW_H;
	}
	double h = FL_TITLE_H + pin_rows * FL_PIN_ROW_H + slot_h + 8.0;
	return Sizef(w, h);
}

// ---------------------------------------------------------------------------
// ForceRefineGraph
// ---------------------------------------------------------------------------
void ForceRefineGraph(Graph& graph, int iters)
{
	const GraphDoc& doc = graph.GetDoc();
	int num_groups = doc.groups.GetCount();

	VectorMap<String, int> node_group;
	for(int gi = 0; gi < num_groups; gi++)
		for(const EntityId& nid : doc.groups[gi].nodes)
			node_group.Add(nid, gi);

	double min_h = 1e18;
	for(const NodeDoc& n : doc.nodes) {
		Sizef sz = FLEstimateNodeSize(n);
		if(sz.cy > 1) min_h = min(min_h, sz.cy);
	}
	double cell_size = (min_h < 1e17) ? clamp(min_h / 10.0, 1.0, 100.0) : 10.0;

	bool verbose = s_force_verbose;
	RLOG("ForceRefineGraph: N=" << doc.nodes.GetCount()
	     << " groups=" << num_groups << " cell=" << cell_size << " iters=" << iters);
	if(verbose)
		VOut(String("ForceRefineGraph: N=") << doc.nodes.GetCount()
		     << " groups=" << num_groups << " cell=" << FI(cell_size)
		     << " iters=" << iters << "\n");

	int N = doc.nodes.GetCount();
	Vector<ForceItem> fitems(N);
	for(int i = 0; i < N; i++) {
		const NodeDoc& n = doc.nodes[i];
		fitems[i].pos = n.pos;
		fitems[i].sz  = FLEstimateNodeSize(n);
		int gi = node_group.Find(n.id);
		fitems[i].group_id = (gi >= 0) ? node_group[gi] : -1;
		int in_c = 0, out_c = 0;
		for(const PinDoc& p : n.pins)
			(p.kind == PinKind::Output ? out_c : in_c)++;
		fitems[i].port_count = max(in_c, out_c);
	}

	// Spread nodes if they are all stacked at the same position
	{
		double sx = 0, sy = 0;
		for(int i = 0; i < N; i++) { sx += fitems[i].pos.x; sy += fitems[i].pos.y; }
		sx /= N; sy /= N;
		double spread = 0;
		for(int i = 0; i < N; i++)
			spread = max(spread, max(fabs(fitems[i].pos.x-sx), fabs(fitems[i].pos.y-sy)));

		if(spread < cell_size * 2) {
			int gc = max(num_groups, 1);
			int gcols = (int)ceil(sqrt((double)gc));
			Vector<int> gcn2(gc, 0);
			for(int i = 0; i < N; i++) {
				int g = fitems[i].group_id;
				if(g >= 0 && g < gc) gcn2[g]++;
			}
			double avg_w = 0, avg_h = 0;
			for(int g = 0; g < gc; g++) {
				int cnt = max(gcn2[g], 1);
				avg_w += 200.0 * cnt + cell_size * 4 * cnt;
				avg_h += 400.0;
			}
			avg_w /= gc; avg_h /= gc;
			double step_x = avg_w * 1.2, step_y = avg_h * 1.5;
			Vector<int> placed(gc, 0);
			for(int i = 0; i < N; i++) {
				int g = fitems[i].group_id;
				if(g < 0 || g >= gc) g = 0;
				int gcol = g % gcols, grow = g / gcols;
				int ng = placed[g]++;
				fitems[i].pos = Pointf(gcol * step_x + ng * (200.0 + cell_size * 4),
				                       grow * step_y);
			}
		}
	}

	ForceRefine(fitems, num_groups, cell_size, iters);

	// Write positions and estimated sizes back
	for(int i = 0; i < N; i++) {
		NodeDoc* n = graph.FindNode(doc.nodes[i].id);
		if(!n) continue;
		n->pos = fitems[i].pos;
		// Write estimated size so Scene.cpp can compute correct group rects
		// before the first paint. Scene::Build will overwrite with real size.
		n->sz = fitems[i].sz;
		graph.Invalidate(n->id);
	}

	if(verbose) {
		// Final report: group bboxes using Rect::Intersects
		String s = "=== Final group bounding boxes ===\n";
		Vector<Rect> grects(num_groups, Rect(0,0,0,0));
		Vector<bool> gvalid(num_groups, false);
		for(int i = 0; i < N; i++) {
			int g = fitems[i].group_id;
			if(g < 0 || g >= num_groups) continue;
			Rect nr((int)fitems[i].pos.x, (int)fitems[i].pos.y,
			        (int)(fitems[i].pos.x + fitems[i].sz.cx),
			        (int)(fitems[i].pos.y + fitems[i].sz.cy));
			if(!gvalid[g]) { grects[g] = nr; gvalid[g] = true; }
			else             grects[g] = grects[g] | nr;
		}
		double dp = GROUP_PAD * cell_size;
		int gov = 0;
		for(int g = 0; g < num_groups; g++) {
			if(!gvalid[g]) continue;
			Rect rp = grects[g].Inflated((int)dp);
			s << "  grp[" << g << "] " << doc.groups[g].id
			  << " raw=(" << grects[g].left << "," << grects[g].top << ")-("
			  << grects[g].right << "," << grects[g].bottom << ") "
			  << grects[g].Width() << "x" << grects[g].Height()
			  << "  padded=(" << rp.left << "," << rp.top << ")-("
			  << rp.right << "," << rp.bottom << ")\n";
		}
		for(int ga = 0; ga < num_groups; ga++) { if(!gvalid[ga]) continue;
			for(int gb = ga+1; gb < num_groups; gb++) { if(!gvalid[gb]) continue;
				Rect ra = grects[ga].Inflated((int)dp);
				Rect rb = grects[gb].Inflated((int)dp);
				if(ra.Intersects(rb)) {
					gov++;
					Rect isect = ra & rb;
					s << "  OVERLAP grp[" << ga << "] " << doc.groups[ga].id
					  << " vs grp[" << gb << "] " << doc.groups[gb].id
					  << " isect=" << isect.Width() << "x" << isect.Height() << "\n";
				}
			}
		}
		s << "=== " << gov << " group overlaps remaining ===\n";
		VOut(s);
	}
}

} // namespace Node

} // namespace Upp
