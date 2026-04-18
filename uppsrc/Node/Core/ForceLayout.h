#ifndef _Node_Core_ForceLayout_h_
#define _Node_Core_ForceLayout_h_

#include "Core.h"

namespace Upp {

namespace Node {

// ---------------------------------------------------------------------------
// ForceLayout: force-directed refinement pass
//
// Portable — depends only on Core types (Pointf, Sizef, Vector<T>).
// Run after any initial placement (SA, grid, spiral, circle) to:
//   - repel overlapping nodes apart
//   - attract each node toward its group centroid  (keeps groups compact)
//   - attract each node toward the global centroid (compresses overall scene)
//   - repel groups from each other (via per-node force)
//
// Minimum inter-node gap = 4 * cell_size + 0.5 * cell_size * max_ports
// so edges have room to route between dense nodes.
// ---------------------------------------------------------------------------

struct ForceItem : Moveable<ForceItem> {
	Pointf pos;        // centre position (read/write)
	Sizef  sz;         // bounding size (read-only)
	int    group_id;   // index into groups array (-1 = ungrouped)
	int    port_count; // max(in_ports, out_ports)
};

struct ForceGroup : Moveable<ForceGroup> {
	// Group membership is encoded via ForceItem::group_id; no extra fields needed.
	// Kept as a placeholder so callers can extend later.
};

// Global verbose flag — set once at startup via SetForceVerbose() or
// by passing -v / --verbose on the command line.
// When true, every ForceRefine iteration prints group rects + overlap counts
// to stdout (works in both GUI and console builds).
void SetForceVerbose(bool v);
bool IsForceVerbose();

// Run force-directed refinement in-place on items[].
void ForceRefine(Vector<ForceItem>& items,
                 int                num_groups,
                 double             cell_size,
                 int                iters = 600);

// Convenience: build ForceItem list from graph nodes, run ForceRefine,
// write positions back.  cell_size is computed from the smallest node dim.
void ForceRefineGraph(Graph& graph, int iters = 600);

} // namespace Node

} // namespace Upp

#endif
