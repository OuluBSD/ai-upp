#include "Umbrella.h"
#include "NavGraph.h"
#include "GameScreen.h"

using namespace Upp;

NavGraph::NavGraph()
	: built(false), cols(0), rows(0),
	  walkableCount(0), componentCount(0), edgeCount(0),
	  gameScreen(nullptr)
{}

// ============================================================================
// Tile queries (identical logic to Pathfinder)
// ============================================================================

bool NavGraph::IsSolid(int col, int row) const {
	if(!gameScreen) return false;
	const Layer* terrain = gameScreen->layerManager.FindLayerByType(LAYER_TERRAIN);
	if(!terrain) return false;
	const MapGrid& g = terrain->GetGrid();
	if(col < 0 || row < 0 || col >= g.GetColumns() || row >= g.GetRows()) return false;
	TileType t = g.GetTile(col, row);
	return t == TILE_WALL || t == TILE_FULLBLOCK;
}

bool NavGraph::IsPassable(int col, int row) const {
	if(!gameScreen) return false;
	const Layer* terrain = gameScreen->layerManager.FindLayerByType(LAYER_TERRAIN);
	if(!terrain) return false;
	const MapGrid& g = terrain->GetGrid();
	if(col < 0 || row < 0 || col >= g.GetColumns() || row >= g.GetRows()) return false;
	TileType t = g.GetTile(col, row);
	return t != TILE_WALL && t != TILE_FULLBLOCK;
}

bool NavGraph::IsWalkable(int col, int row) const {
	if(!IsPassable(col, row)) return false;
	if(!IsPassable(col, row - 1)) return false;
	return IsSolid(col, row + 1);
}

// ============================================================================
// Edge test: can a player move from (fc,fr) to (tc,tr) in one motion?
// Mirrors Pathfinder::AddNeighbors logic.
// ============================================================================

bool NavGraph::HasEdge(int fc, int fr, int tc, int tr) const {
	if(!IsWalkable(fc, fr) || !IsWalkable(tc, tr)) return false;
	int dh = fr - tr;  // positive = going up
	int dw = tc - fc;  // signed horizontal

	// Walk: same row, one tile
	if(dh == 0 && (dw == 1 || dw == -1)) return true;

	// Jump: going up
	if(dh >= 1 && dh <= Pathfinder::MAX_JUMP_HEIGHT) {
		if(abs(dw) <= Pathfinder::MAX_JUMP_WIDTH) {
			// Check head clearance at destination
			if(IsPassable(tc, tr - 1)) return true;
		}
	}

	// Fall: going down, at most 1 tile sideways
	if(dh <= -1 && dh >= -Pathfinder::MAX_FALL_DEPTH) {
		if(abs(dw) <= 1) {
			// Verify fall column is clear between fr+1 and tr
			bool clear = true;
			for(int r = fr + 1; r < tr; r++) {
				if(!IsPassable(fc, r)) { clear = false; break; }
			}
			if(clear) return true;
		}
	}

	return false;
}

// ============================================================================
// BFS flood-fill: assigns compId to all tiles reachable from (startCol,startRow)
// ============================================================================

void NavGraph::FloodFill(int startCol, int startRow, int compId) {
	// BFS queue stored as flat indices
	Vector<int> queue;
	queue.Add(CellIdx(startCol, startRow));
	componentId[CellIdx(startCol, startRow)] = compId;

	int head = 0;
	while(head < queue.GetCount()) {
		int idx = queue[head++];
		int c   = idx % cols;
		int r   = idx / cols;

		// Walk left/right
		auto TryNeighbor = [&](int nc, int nr) {
			if(!InBounds(nc, nr)) return;
			if(componentId[CellIdx(nc, nr)] >= 0) return;  // already visited
			if(!IsWalkable(nc, nr)) return;
			if(!HasEdge(c, r, nc, nr)) return;
			componentId[CellIdx(nc, nr)] = compId;
			queue.Add(CellIdx(nc, nr));
			edgeCount++;
		};

		// Walk
		TryNeighbor(c - 1, r);
		TryNeighbor(c + 1, r);

		// Jump
		for(int dh = 1; dh <= Pathfinder::MAX_JUMP_HEIGHT; dh++) {
			for(int dw = -Pathfinder::MAX_JUMP_WIDTH; dw <= Pathfinder::MAX_JUMP_WIDTH; dw++) {
				TryNeighbor(c + dw, r - dh);
			}
		}

		// Fall
		for(int dd = 1; dd <= Pathfinder::MAX_FALL_DEPTH; dd++) {
			for(int dw = -1; dw <= 1; dw++) {
				TryNeighbor(c + dw, r + dd);
			}
		}
	}
}

// ============================================================================
// Build
// ============================================================================

void NavGraph::Build(const GameScreen* screen) {
	gameScreen     = screen;
	built          = false;
	walkableCount  = 0;
	componentCount = 0;
	edgeCount      = 0;

	if(!screen) return;

	const Layer* terrain = screen->layerManager.FindLayerByType(LAYER_TERRAIN);
	if(!terrain) return;

	const MapGrid& g = terrain->GetGrid();
	cols = g.GetColumns();
	rows = g.GetRows();

	componentId.SetCount(cols * rows, INVALID_COMPONENT);

	// Count walkable tiles
	for(int r = 0; r < rows; r++)
		for(int c = 0; c < cols; c++)
			if(IsWalkable(c, r))
				walkableCount++;

	// Flood-fill connected components
	for(int r = 0; r < rows; r++) {
		for(int c = 0; c < cols; c++) {
			if(IsWalkable(c, r) && componentId[CellIdx(c, r)] == INVALID_COMPONENT) {
				FloodFill(c, r, componentCount);
				componentCount++;
			}
		}
	}

	built = true;
	LOG("NavGraph: built " << walkableCount << " walkable tiles, "
	    << componentCount << " components, " << edgeCount << " edges");
}

// ============================================================================
// Queries
// ============================================================================

int NavGraph::GetComponentId(int col, int row) const {
	if(!built || !InBounds(col, row)) return INVALID_COMPONENT;
	return componentId[CellIdx(col, row)];
}

bool NavGraph::IsReachable(int c1, int r1, int c2, int r2) const {
	if(!built) return false;
	int id1 = GetComponentId(c1, r1);
	int id2 = GetComponentId(c2, r2);
	if(id1 == INVALID_COMPONENT || id2 == INVALID_COMPONENT) return false;
	return id1 == id2;
}
