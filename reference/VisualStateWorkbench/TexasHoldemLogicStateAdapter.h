#ifndef _VisualStateWorkbench_TexasHoldemLogicStateAdapter_h_
#define _VisualStateWorkbench_TexasHoldemLogicStateAdapter_h_

// ---------------------------------------------------------------------------
// TexasHoldem logic-state timeline adapter (task 0133 / M06-03).
//
// GUI-independent (NO CtrlLib / Ctrl dependency): computes, for a loaded
// TexasHoldem M01/M02 session (task 0131's VsmTexasHoldemSession) plus a
// `.form` path, the full per-frame derived TexasHoldemLogicState timeline —
// the exact same derivation `reference/VisualStateLogicCompare`'s CLI
// computes. This does NOT re-implement any recognition/derivation logic: it
// calls `VsmDeriveSessionLogicStates` (uppsrc/VisualStateLogic/
// LogicCompare.h, task 0133's extraction of the CLI's own M05 scoring/
// derivation code), the same shared library the CLI itself now calls, so a
// headless cross-check can agree with the CLI byte-for-byte on every derived
// field (see this task's evidence section).
//
// Unlike task 0132's layout-binding adapter (which reuses the session's
// ALREADY-detected region nodes for exact 1:1 alignment), this adapter
// re-runs its own frame decode + change-detection pass internally (inside
// VsmDeriveSessionLogicStates, reading straight from the session directory
// on disk) rather than reusing VsmTexasHoldemSession::regions — the M05
// derivation pipeline needs per-role TEMPLATE-MATCH SCORING against actual
// frame pixels (puck/card/action-icon/hole-card recognition), which task
// 0131's generic region adapter never computed, so there is nothing to reuse
// there beyond the session root path itself. This is the same shape
// `reference/VisualStateLogicCompare/main.cpp` itself uses (it also decodes
// frames directly from the session directory, independent of any adapter).
//
// Being CtrlLib-free, this whole adapter can be exercised from a
// CONSOLE/GUI_APP_MAIN harness (the project's "Headless/GUI Dual-Purpose
// Rule"), which is how it is verified — the GUI panel that consumes it
// cannot be driven by a background agent.
// ---------------------------------------------------------------------------

#include "TexasHoldemSessionAdapter.h"
#include <VisualStateLogic/LogicCompare.h>

namespace Upp {

// One fully-derived logic-state timeline for a session: one
// VsmLogicCompareRecordOut per ground-truth frame, in frame_id order. Built
// once per opened session + resolved `.form` path (the same "build once,
// index per frame" shape task 0132's VsmSessionLayoutModel uses).
struct VsmSessionLogicModel {
	bool   loaded = false;
	String error;
	String form_path;

	Vector<VsmLogicCompareRecordOut> records; // records[i].frame_id == i

	bool IsEmpty() const { return !loaded; }

	// Record for frame_id, or nullptr if out of range / not loaded.
	const VsmLogicCompareRecordOut* RecordForFrame(int frame_id) const;
};

// Build the full per-frame logic-state timeline for `session` from
// `form_path`, by calling VsmDeriveSessionLogicStates(session.root,
// form_path, ...) — the shared M05 derivation pipeline extracted (task 0133)
// from reference/VisualStateLogicCompare/main.cpp. On failure sets
// `.loaded=false` and `.error`. Purely headless; can be slow (the same
// template-match scoring cost the CLI has) for longer sessions.
VsmSessionLogicModel VsmBuildSessionLogicModel(const VsmTexasHoldemSession& session,
                                               const String& form_path);

} // namespace Upp

#endif
