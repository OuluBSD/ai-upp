#ifndef _VisualStateWorkbench_TexasHoldemSessionAdapter_h_
#define _VisualStateWorkbench_TexasHoldemSessionAdapter_h_

// ---------------------------------------------------------------------------
// TexasHoldem M01/M02 session adapter (task 0131 / M06-01).
//
// GUI-independent (NO CtrlLib / Ctrl dependency): bridges a real TexasHoldem
// `--record-session` output directory (metadata.json + groundtruth.jsonl +
// frames/%08d.png) into the plain, generic VisualStateModel data types the
// existing VisualStateWorkbench dock panels already know how to display
// (VsmRegionNode / VsmChangedRect / VsmM01M02SessionInfo). Every piece of
// per-frame/region logic here is a direct reuse of the exact library functions
// reference/VisualStateRegionDump's M01/M02 code path uses
// (VsmReadM01M02SessionInfo / VsmLoadM01M02SessionFrame / VsmDetectChanges /
// VsmRegionMemory), with the same change-detection parameters — nothing about
// the change-detection or region-identity algorithm is re-derived here.
//
// Because it depends only on VisualStateModel + game/TexasHoldem (for the
// ground-truth record type) and NOT on CtrlLib, this whole adapter can be
// exercised headlessly from a CONSOLE/GUI_APP_MAIN harness, per the project's
// "Headless/GUI Dual-Purpose Rule".
// ---------------------------------------------------------------------------

#include <VisualStateModel/VisualStateModel.h>
#include <TexasHoldem/TexasHoldemSessionContract.h>

namespace Upp {

// One fully-loaded TexasHoldem session, in the workbench's generic shapes.
struct VsmTexasHoldemSession {
	String                                root;
	VsmM01M02SessionInfo                  info;

	// One VsmRegionNode per changed region per frame transition — the same
	// per-transition granularity VisualStateRegionDump emits as JSONL records.
	// Fields: stable id (rgn-%04d, shared across transitions via fingerprint
	// nearest-match), target frame id, rect (x/y/w/h in table pixels), action
	// ("created" for a freshly-assigned id, "moved" for a matched existing id),
	// and the 32x32 fingerprint hash. Ordered by (frame, detection order).
	Vector<VsmRegionNode>                 regions;

	// Ground-truth records densely indexed by frame_id. Size is at least
	// info.frame_count; a slot with no matching groundtruth.jsonl line is left
	// default-constructed (frame_id == 0). Consumed by later M06 tasks.
	Vector<TexasHoldemGroundTruthRecord>  ground_truth;

	int  distinct_region_count = 0; // number of distinct stable region ids
	int  ground_truth_count    = 0; // number of groundtruth.jsonl lines parsed
	bool loaded                = false;

	bool IsEmpty() const { return !loaded; }

	// Region nodes whose .frame == frame_id, in stable detection order.
	// Returned as pointers into `regions` (valid as long as this session is not
	// reassigned / cleared).
	Vector<const VsmRegionNode*> RegionsForFrame(int frame_id) const;

	// Ground-truth record for frame_id, or nullptr if out of range.
	const TexasHoldemGroundTruthRecord* GroundTruthForFrame(int frame_id) const;
};

struct VsmTexasHoldemLoadResult {
	bool   success              = false;
	String error;
	int    frame_count          = 0;
	int    transitions          = 0; // consecutive-frame pairs processed
	int    region_records       = 0; // == out.regions.GetCount()
	int    distinct_regions     = 0;
	int    ground_truth_records = 0;
};

// Load a TexasHoldem M01/M02 session directory into `out`. Reads metadata.json,
// parses groundtruth.jsonl, and runs change detection + region identity across
// every consecutive frame pair (same params as VisualStateRegionDump). Purely
// headless. `log` is optional (forwarded to VsmRegionMemory diagnostics).
VsmTexasHoldemLoadResult VsmLoadTexasHoldemSession(const String& root,
                                                   VsmTexasHoldemSession& out,
                                                   AppLog* log = nullptr);

// Decode frames/%08d.png for the given frame_id (thin wrapper over
// VsmLoadM01M02SessionFrame so callers need only the session object).
bool VsmLoadTexasHoldemFrameImage(const VsmTexasHoldemSession& s, int frame_id,
                                  VsmFrameImage& out);

} // namespace Upp

#endif
