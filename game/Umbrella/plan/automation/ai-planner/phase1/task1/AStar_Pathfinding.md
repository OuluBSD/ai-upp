# Task: A* Pathfinding Implementation

## Overview
Implement A* pathfinding algorithm for Umbrella game to enable AI-controlled player navigation through levels.

## Objective
Create a pathfinding system that can find optimal paths from player's current position to goal positions, accounting for:
- Level geometry (walls, blocks, platforms)
- Jump mechanics (can reach higher platforms)
- Fall mechanics (can drop down safely)

## Prerequisites
- **Foundation Track Phase 2** complete: Player/enemy state API exposed
- Understanding of A* algorithm
- Familiarity with Umbrella level structure (MapGrid, TileType)
- Knowledge of player physics constants (JUMP_VELOCITY, MOVE_SPEED, GRAVITY)

## Background

**A* Algorithm Overview:**
```
function A*(start, goal):
    openSet = {start}
    cameFrom = {}
    gScore[start] = 0
    fScore[start] = heuristic(start, goal)

    while openSet not empty:
        current = node in openSet with lowest fScore
        if current == goal:
            return reconstruct_path(cameFrom, current)

        openSet.remove(current)
        closedSet.add(current)

        for neighbor in neighbors(current):
            tentative_gScore = gScore[current] + distance(current, neighbor)
            if neighbor in closedSet and tentative_gScore >= gScore[neighbor]:
                continue
            if tentative_gScore < gScore[neighbor] or neighbor not in openSet:
                cameFrom[neighbor] = current
                gScore[neighbor] = tentative_gScore
                fScore[neighbor] = gScore[neighbor] + heuristic(neighbor, goal)
                openSet.add(neighbor)

    return failure
```

**Umbrella-Specific Considerations:**
- Grid-based movement (14 px tiles)
- Jump arcs (can reach ~4 tiles high with JUMP_VELOCITY=280)
- Walk off ledges (need safe landing check)
- Enemy avoidance (higher cost for tiles near enemies)

## Implementation Steps

### Step 1: Create PathNode Structure
**File**: `game/Umbrella/PathNode.h`

```cpp
#ifndef _Umbrella_PathNode_h_
#define _Umbrella_PathNode_h_

#include <Core/Core.h>

using namespace Upp;

enum MoveType {
    MOVE_WALK,       // Horizontal walk
    MOVE_JUMP,       // Jump to higher location
    MOVE_FALL,       // Drop down to lower location
    MOVE_GLIDE       // Glide to extend horizontal distance
};

struct PathNode {
    int col, row;          // Grid coordinates
    float gScore;          // Cost from start
    float fScore;          // gScore + heuristic
    PathNode* parent;      // Previous node in path
    MoveType moveType;     // How we got here

    PathNode() : col(0), row(0), gScore(0), fScore(0), parent(nullptr), moveType(MOVE_WALK) {}
    PathNode(int c, int r) : col(c), row(r), gScore(0), fScore(0), parent(nullptr), moveType(MOVE_WALK) {}

    bool operator==(const PathNode& other) const {
        return col == other.col && row == other.row;
    }

    dword GetHashValue() const {
        return CombineHash(col, row);
    }
};

#endif
```

### Step 2: Create Pathfinder Class
**File**: `game/Umbrella/Pathfinder.h`

```cpp
#ifndef _Umbrella_Pathfinder_h_
#define _Umbrella_Pathfinder_h_

#include "PathNode.h"
#include "MapGrid.h"
#include "GameScreen.h"

using namespace Upp;

class Pathfinder {
private:
    const GameScreen* gameScreen;  // Access to level and entities

    // A* internal state
    Array<PathNode> nodePool;  // Memory pool for path nodes
    VectorMap<Point, PathNode*> openSet;
    VectorMap<Point, PathNode*> closedSet;

public:
    Pathfinder();
    void SetGameScreen(const GameScreen* screen) { gameScreen = screen; }

    // Find path from start to goal
    Vector<PathNode> FindPath(Point start, Point goal);

    // Helper methods
    Vector<PathNode> GetNeighbors(const PathNode& node);
    float Heuristic(Point a, Point b);
    float MoveCost(const PathNode& from, const PathNode& to);
    bool IsWalkable(int col, int row);
    bool CanJumpTo(Point from, Point to);
    bool CanFallTo(Point from, Point to);

private:
    Vector<PathNode> ReconstructPath(PathNode* goal);
};

#endif
```

### Step 3: Implement Core A* Algorithm
**File**: `game/Umbrella/Pathfinder.cpp`

```cpp
#include "Umbrella.h"
#include "Pathfinder.h"

Pathfinder::Pathfinder() {
    gameScreen = nullptr;
}

Vector<PathNode> Pathfinder::FindPath(Point start, Point goal) {
    if(!gameScreen) {
        LOG("ERROR: GameScreen not set");
        return Vector<PathNode>();
    }

    // Clear previous state
    nodePool.Clear();
    openSet.Clear();
    closedSet.Clear();

    // Create start node
    PathNode* startNode = &nodePool.Add();
    startNode->col = start.x;
    startNode->row = start.y;
    startNode->gScore = 0;
    startNode->fScore = Heuristic(start, goal);
    startNode->parent = nullptr;

    openSet.Add(start, startNode);

    while(openSet.GetCount() > 0) {
        // Find node with lowest fScore
        PathNode* current = nullptr;
        float lowestF = 1e9;
        Point currentPoint;

        for(int i = 0; i < openSet.GetCount(); i++) {
            if(openSet[i]->fScore < lowestF) {
                lowestF = openSet[i]->fScore;
                current = openSet[i];
                currentPoint = Point(current->col, current->row);
            }
        }

        // Check if reached goal
        if(currentPoint == goal) {
            return ReconstructPath(current);
        }

        // Move current from open to closed
        openSet.RemoveKey(currentPoint);
        closedSet.Add(currentPoint, current);

        // Explore neighbors
        Vector<PathNode> neighbors = GetNeighbors(*current);
        for(PathNode& neighbor : neighbors) {
            Point neighborPoint(neighbor.col, neighbor.row);

            // Skip if in closed set
            if(closedSet.Find(neighborPoint) >= 0) {
                continue;
            }

            float tentativeG = current->gScore + MoveCost(*current, neighbor);

            // Get or create neighbor node
            PathNode* neighborNode = nullptr;
            int idx = openSet.Find(neighborPoint);
            if(idx >= 0) {
                neighborNode = openSet[idx];
                if(tentativeG >= neighborNode->gScore) {
                    continue;  // Not a better path
                }
            }
            else {
                neighborNode = &nodePool.Add(neighbor);
                openSet.Add(neighborPoint, neighborNode);
            }

            // Update neighbor
            neighborNode->parent = current;
            neighborNode->gScore = tentativeG;
            neighborNode->fScore = tentativeG + Heuristic(neighborPoint, goal);
        }
    }

    // No path found
    LOG("WARNING: No path found from " << start << " to " << goal);
    return Vector<PathNode>();
}

Vector<PathNode> Pathfinder::ReconstructPath(PathNode* goal) {
    Vector<PathNode> path;
    PathNode* current = goal;

    while(current) {
        path.Insert(0, *current);  // Prepend (reverse order)
        current = current->parent;
    }

    return path;
}
```

### Step 4: Implement Neighbor Generation
**File**: `game/Umbrella/Pathfinder.cpp`

```cpp
Vector<PathNode> Pathfinder::GetNeighbors(const PathNode& node) {
    Vector<PathNode> neighbors;
    Point current(node.col, node.row);

    // Walk left
    Point left(node.col - 1, node.row);
    if(IsWalkable(left.x, left.y)) {
        PathNode n(left.x, left.y);
        n.moveType = MOVE_WALK;
        neighbors.Add(n);
    }

    // Walk right
    Point right(node.col + 1, node.row);
    if(IsWalkable(right.x, right.y)) {
        PathNode n(right.x, right.y);
        n.moveType = MOVE_WALK;
        neighbors.Add(n);
    }

    // Jump up (check multiple heights: 1-4 tiles)
    for(int h = 1; h <= 4; h++) {
        // Jump up-left
        Point jumpUpLeft(node.col - 1, node.row - h);
        if(CanJumpTo(current, jumpUpLeft)) {
            PathNode n(jumpUpLeft.x, jumpUpLeft.y);
            n.moveType = MOVE_JUMP;
            neighbors.Add(n);
        }

        // Jump up-right
        Point jumpUpRight(node.col + 1, node.row - h);
        if(CanJumpTo(current, jumpUpRight)) {
            PathNode n(jumpUpRight.x, jumpUpRight.y);
            n.moveType = MOVE_JUMP;
            neighbors.Add(n);
        }
    }

    // Fall down (check multiple depths: 1-10 tiles)
    for(int d = 1; d <= 10; d++) {
        // Fall down-left
        Point fallLeft(node.col - 1, node.row + d);
        if(CanFallTo(current, fallLeft)) {
            PathNode n(fallLeft.x, fallLeft.y);
            n.moveType = MOVE_FALL;
            neighbors.Add(n);
            break;  // Only add first valid landing
        }

        // Fall down-right
        Point fallRight(node.col + 1, node.row + d);
        if(CanFallTo(current, fallRight)) {
            PathNode n(fallRight.x, fallRight.y);
            n.moveType = MOVE_FALL;
            neighbors.Add(n);
            break;  // Only add first valid landing
        }
    }

    return neighbors;
}
```

### Step 5: Implement Helper Methods
**File**: `game/Umbrella/Pathfinder.cpp`

```cpp
float Pathfinder::Heuristic(Point a, Point b) {
    // Manhattan distance
    return (float)(abs(a.x - b.x) + abs(a.y - b.y));
}

float Pathfinder::MoveCost(const PathNode& from, const PathNode& to) {
    float baseCost = Heuristic(Point(from.col, from.row), Point(to.col, to.row));

    // Penalty for movement type
    switch(to.moveType) {
        case MOVE_WALK: baseCost *= 1.0f; break;
        case MOVE_JUMP: baseCost *= 1.5f; break;  // Jumps are harder
        case MOVE_FALL: baseCost *= 1.2f; break;  // Falls are risky
        case MOVE_GLIDE: baseCost *= 1.3f; break;
    }

    // TODO: Add enemy proximity penalty
    // TODO: Add danger zone penalty (spikes, projectiles)

    return baseCost;
}

bool Pathfinder::IsWalkable(int col, int row) {
    if(!gameScreen) return false;

    const LayerManager& layerMgr = gameScreen->GetLayerManager();
    const Layer* terrain = layerMgr.FindLayerByType(LAYER_TERRAIN);
    if(!terrain) return false;

    const MapGrid& grid = terrain->GetGrid();

    // Out of bounds
    if(col < 0 || row < 0 || col >= grid.GetColumns() || row >= grid.GetRows()) {
        return false;
    }

    // Check if tile is empty (not wall or block)
    TileType tile = grid.GetTile(col, row);
    if(tile == TILE_WALL || tile == TILE_FULLBLOCK) {
        return false;
    }

    // Check if there's ground below (need something to stand on)
    if(row + 1 < grid.GetRows()) {
        TileType below = grid.GetTile(col, row + 1);
        if(below == TILE_EMPTY) {
            return false;  // Would fall through
        }
    }

    return true;
}

bool Pathfinder::CanJumpTo(Point from, Point to) {
    // Check if vertical distance is within jump range
    int heightDiff = from.y - to.y;  // Negative if jumping up
    if(heightDiff > 0 || heightDiff < -4) {
        return false;  // Can't jump down or too high
    }

    // Check if horizontal distance is reachable
    int horizDist = abs(to.x - from.x);
    if(horizDist > 2) {
        return false;  // Too far horizontally
    }

    // Check if destination is walkable
    return IsWalkable(to.x, to.y);
}

bool Pathfinder::CanFallTo(Point from, Point to) {
    // Check if falling down
    int heightDiff = to.y - from.y;
    if(heightDiff <= 0) {
        return false;  // Not falling
    }

    // Check if horizontal offset is reachable while falling
    int horizDist = abs(to.x - from.x);
    if(horizDist > heightDiff / 2) {
        return false;  // Too far to drift
    }

    // Check if destination is walkable
    return IsWalkable(to.x, to.y);
}
```

### Step 6: Create Test Script
**File**: `game/Umbrella/tests/ai/test_pathfinding.py`

```python
# Test A* pathfinding

print("Testing pathfinding...")

# Load level
load_level("world1-stage1.json")

# Set player at start position
player_start = [5, 20]
set_player_position(player_start[0] * 14, player_start[1] * 14)

# Find path to goal
goal = [30, 15]
path = find_path(player_start, goal)

print("Path found with", len(path), "nodes")

if len(path) > 0:
    for i, node in enumerate(path):
        print(f"  {i}: ({node['col']}, {node['row']}) - {node['move_type']}")

    # Verify path reaches goal
    last_node = path[-1]
    assert last_node['col'] == goal[0], "Path doesn't reach goal X"
    assert last_node['row'] == goal[1], "Path doesn't reach goal Y"

    print("Pathfinding test passed!")
else:
    print("ERROR: No path found")
```

## Testing

### Build Test
```bash
script/build.py -mc 1 -j 12 Umbrella
```

### Unit Test (C++)
Create `PathfinderTest.cpp` to test algorithm in isolation:
```cpp
void test_pathfinding() {
    GameScreen screen;
    screen.LoadLevel("share/mods/umbrella/levels/world1-stage1.json");

    Pathfinder pathfinder;
    pathfinder.SetGameScreen(&screen);

    Point start(5, 20);
    Point goal(30, 15);

    Vector<PathNode> path = pathfinder.FindPath(start, goal);

    ASSERT(path.GetCount() > 0);
    ASSERT(path[path.GetCount() - 1].col == goal.x);
    ASSERT(path[path.GetCount() - 1].row == goal.y);
}
```

### Integration Test (Python)
```bash
bin/Umbrella --test game/Umbrella/tests/ai/test_pathfinding.py
```

## Success Criteria
- [ ] PathNode and Pathfinder classes implemented
- [ ] A* algorithm finds valid paths in simple levels
- [ ] Jump arcs correctly handled (up to 4 tiles)
- [ ] Fall mechanics correctly handled
- [ ] Test script passes
- [ ] Performance: < 100ms for typical paths

## Known Issues / Gotchas
1. **Jump Arc Physics**: May need to refine CanJumpTo() with actual physics simulation
2. **Performance**: A* can be slow on large levels - consider heuristic tuning or jump point search
3. **Enemy Avoidance**: Needs dynamic replanning when enemies move
4. **One-Way Platforms**: May need special handling for TILE_WALL

## Next Steps
- **Phase 1 Task 2**: Build level navigation graph (precompute walkable areas)
- **Phase 1 Task 3**: Movement cost calculation (add danger zones, enemy proximity)
- **Phase 2 Task 1**: Action primitive implementation

## References
- A* Algorithm: https://en.wikipedia.org/wiki/A*_search_algorithm
- `game/Umbrella/Player.h`: Jump mechanics (JUMP_VELOCITY = 280)
- `game/Umbrella/MapGrid.h`: Level tile structure
