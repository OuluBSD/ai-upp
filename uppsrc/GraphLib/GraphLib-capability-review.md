# GraphLib Capability Review

## 1. Executive Summary
`GraphLib` today is a hybrid of two layers: a classic graph visualization/layout library (`Graph`, `GraphLayout<T>`, `Renderer`, layout algorithms) and a newer but still incomplete node-editor-oriented control (`GraphNodeCtrl`). The core graph model, basic drawing, and several automatic layouts are implemented and usable. Tutorials `GraphLib1..4` exercise this classic layer (tree/ordered/spring layouts and Dijkstra highlighting).

At the same time, many “modern node editor” expectations are only partially implemented or missing: full load/deserialization is missing (`GraphNodeCtrl::LoadGraph` is a stub), undo/redo is absent, edge interaction is incomplete, child widgets inside nodes are not supported, and several interaction/performance systems are ad hoc. There are also signs of unfinished migration work (unused/dead declarations, stale pseudo-code headers like `Raphael.h`/`Snap.h`, and partially wired animation/pan systems).

Overall maturity: **functional baseline graph library with partial editor features**, best described as an **early-to-mid prototype layer** rather than a production-grade node editor framework.

## 2. Package Overview
### `./tutorial/GraphLib1/`
- Purpose: basic binary-tree style graph demo.
- Main files: `GraphLib1.upp`, `GraphLib1.h`, `main.cpp`.
- Demonstrates: `TournamentTreeGraph`, integer node IDs via `AddEdge(int,int)`, default coloring via `SetFillColor`.
- Important API touchpoints: `TournamentTreeGraph`, `Graph::SetFillColor`, `Graph::AddEdge`.

### `./tutorial/GraphLib2/`
- Purpose: dependency graph demo for ordered/tree-like layering.
- Main files: `GraphLib2.upp`, `GraphLib2.h`, `main.cpp`.
- Demonstrates: `OrderedTreeGraph`, string node IDs, implicit node creation via `AddEdge`.
- Important API touchpoints: `OrderedTreeGraph`, `AddNode`, `AddEdge`.

### `./tutorial/GraphLib3/`
- Purpose: style + directed-edge + spring-layout demo.
- Main files: `GraphLib3.upp`, `GraphLib3.h`, `main.cpp`.
- Demonstrates: `SpringGraph`, node shape/styling (`SetShapeDiamond`, `SetShapeRectangle`, `SetFill`), edge labels/strokes/direction.
- Important API touchpoints: `SpringGraph`, `Node` style setters, `Edge` style setters.

### `./tutorial/GraphLib4/`
- Purpose: shortest-path highlighting demo.
- Main files: `GraphLib4.upp`, `GraphLib4.h`, `main.cpp`.
- Demonstrates: weighted edges, `RouteWeights()`, `Dijkstra()` route highlighting and relabeling.
- Important API touchpoints: `GraphLayout<T>::RouteWeights`, `GraphLayout<T>::Dijkstra`.

### `./uppsrc/GraphLib/`
- Purpose: implementation package.
- Main files:
  - Model: `Graph.h`, `Graph.cpp`, `GroupNode.h`.
  - View/Rendering: `Renderer.h`, `Renderer.cpp`, `GraphLib.h` (`GraphLayout<T>` control).
  - Interaction/editor control: `GraphNodeCtrl.h`, `GraphNodeCtrl.cpp`.
  - Layout/routing algorithms: `Layout.*`, `Spring.*`, `OrderedTree.*`, `TournamentTree.*`, `TopologicalSort.*`, `Dijkstra.*`, `BinaryHeapMap.*`.
  - Package metadata: `GraphLib.upp`.
- Important classes/functions:
  - `Graph`, `Node`, `Pin`, `Edge`, `GroupNode`.
  - `GraphLayout<T>` (`SpringGraph`, `OrderedTreeGraph`, `TournamentTreeGraph`).
  - `Renderer::SetImageSize`, `Renderer::Paint`, hit-test helpers.
  - `GraphNodeCtrl` interaction surface (selection, context menus, copy/paste, linking).

## 3. Feature Matrix
| Capability | Status | Evidence | Notes / Limitations |
|---|---|---|---|
| graph model | Present | `uppsrc/GraphLib/Graph.h` (`class Graph`), `Graph.cpp` CRUD methods | Mutable in-memory model exists. |
| node model | Present | `Graph.h` (`struct Node`) | Mixes logical, layout, visual, selection, animation state in one struct. |
| ports / pins | Present | `Graph.h` (`struct Pin`, `Node::AddPin`, `FindPin`), `Graph.cpp` | Pin model has type/kind/color/size and edge references. |
| edges / connections | Present | `Graph.h` (`struct Edge`), `Graph.cpp` (`AddEdge`, `RemoveEdge`) | Supports weighted, directed flags and labels. |
| grouping / containers / comments | Partial | `GroupNode.h`, `Graph::groups`, `MoveNodeToGroup` | Group container exists; comment-node concept absent. Collapse flag not functionally applied in rendering logic. |
| serialization / deserialization | Partial | `GraphNodeCtrl::SaveNodePositions`, `SaveGraph`; `LoadGraph` stub in `GraphNodeCtrl.cpp` | Save implemented (custom text format, not XML despite file names); full load missing. |
| separation between logical model and visual state | Partial | `Node` includes `layout_pos_*`, `point`, colors, selection, animation (`Graph.h`) | Strong coupling of model and view/edit state. |
| node boxes | Present | `Renderer::Paint` draws rect/ellipse/diamond | Basic primitives only. |
| titles / captions | Present | `Node.label` and `DrawText` in `Renderer::Paint` | Single text line centering; no rich text/format controls. |
| port rendering | Present | `Renderer::Paint` pin rectangles, `Renderer::FindPin` | Pins are simple squares. |
| edge rendering | Present | `Renderer.cpp` (`DrawCurvedEdge`, straight pin-edge fallback) | Bezier-like polyline for normal edges; straight line for pin-connected edges. |
| labels on edges | Present | `DrawCurvedEdge` and pin-edge midpoint text in `Renderer::Paint` | Basic text only. |
| comment/group boxes | Partial | `GroupNode` + group drawing in `Renderer::Paint` | Group visuals exist; no dedicated comment object/system. |
| previews / thumbnails | Absent | No preview/thumbnail APIs in `GraphLib` package | None found in model or renderer. |
| overlays, selection visuals, minimap, annotations | Partial | Selection highlight in `Renderer::Paint`; box-select overlay in `GraphNodeCtrl::Paint` | No minimap, no annotation layer. |
| node move | Present | `GraphNodeCtrl::MouseMove` updates `dragNode->point` | Direct point mutation; not command-based. |
| multi-selection | Present | Ctrl-click and selected vectors in `GraphNodeCtrl::LeftDown` | Works for nodes/groups; edge selection interaction is weak. |
| box selection / marquee | Present | `isBoxSelecting`, rectangle logic in `LeftUp` | Implemented for nodes and groups. |
| drag-connect between ports | Present | `StartLinkCreation`, `LeftUp` pin-to-pin edge creation | Basic validation only (input/output mismatch check). |
| edge selection | Partial | `SelectEdge` exists; hover detection in `MouseMove`; no clear click-select path | Right-click edge menu currently depends on already-selected edges. |
| edge rerouting | Absent | No reroute handles or interaction path | Routing is automatic per paint only. |
| resize | Absent | No node/group resize drag handlers | Group size can be set on creation; not interactively resized. |
| collapse/expand | Partial | `GroupNode::is_collapsed`; context menu toggles it | No actual collapse behavior applied to node visibility/layout. |
| context menu | Present | `GraphNodeCtrl::RightDown` menus for pin/node/group/background | Rich enough for basic editing commands. |
| keyboard shortcuts | Present | `GraphNodeCtrl::Key` (copy/paste/select-all/delete/focus/save/load/etc.) | No undo/redo shortcuts. |
| clipboard copy/paste | Partial | `CopyNodes` / `PasteNodes` in `GraphNodeCtrl.cpp` | Internal memory clipboard only, not OS clipboard; edge/pin mapping is heuristic. |
| undo/redo | Absent | No command history stack in package | Not implemented. |
| buttons inside nodes | Absent | No child `Ctrl` hosting per node | Nodes are custom-painted shapes. |
| edit fields inside nodes | Absent | No node-embedded editor controls | Not present. |
| sliders inside nodes | Absent | No support in model/controller | Not present. |
| toggles inside nodes | Absent | No support in model/controller | Not present. |
| focus handling | Partial | Control-level keyboard/mouse handling in `GraphNodeCtrl` | No per-node widget focus model. |
| text editing without breaking graph interaction | Absent | No inline node text-edit control logic | Labels are passive text. |
| generic host/container model for child controls | Absent | No API for node-owned child `Ctrl`s | None in `GraphNodeCtrl`/`Renderer`/`Node`. |
| automatic node layout | Present | `GraphLayout<T>::RefreshLayout()` delegates to layout algorithms | Exists for graph-level control. |
| layered / DAG layout | Partial | `OrderedTree` + `TopologicalSort` | Tree/DAG-like assumptions; not a full Sugiyama/orthogonal layered router. |
| tree layout | Present | `TournamentTree`, `OrderedTree` | Implemented with specific assumptions. |
| force-directed layout | Present | `Spring` layout implementation | Basic spring model. |
| grid snapping | Absent | `Snap.h` is stale non-C++ pseudo-code and not in `GraphLib.upp` file list | No active snap system in compiled package. |
| alignment guides | Absent | No guide rendering or snapping APIs | Not found. |
| orthogonal routing | Absent | Edge drawing uses straight or bezier-like polyline | No Manhattan router. |
| bezier routing | Present | `DrawCurvedEdge` in `Renderer.cpp` | Approximate Bezier via sampled polyline. |
| path recomputation | Present | Edge paths recomputed during each paint from current node positions | No incremental path cache. |
| zoom / pan | Partial | `GraphLayout<T>` zoom+scrollbars (`GraphLib.h`); `GraphNodeCtrl` has separate `zoomFactor/panOffset` path | `GraphNodeCtrl` zoom/pan state appears only partially wired to rendering model. |
| viewport clipping / culling | Partial | `GraphLayout<T>::Paint` draws image sub-rect (`DrawImage` with source rect) | Clipping via cached image viewport; no object-level culling. |
| partial redraw | Partial | Cached full-scene image (`One<ImageDraw> id`) in `GraphLayout<T>` | Uses full buffer regenerate in `RefreshBuffer`; not fine-grained invalidation. |
| caching | Present | Cached image buffer in `GraphLayout<T>` | Coarse cache only. |
| hit testing strategy | Present | `Renderer::FindNode/FindPin/FindGroup`; edge proximity in `GraphNodeCtrl::MouseMove` | Mostly linear scans; edge hit test only for hover. |
| large graph behavior | Partial | Zoom guard constants in `RefreshBuffer`; O(N²) collision checks, full-scene redraw | Some safety limits exist, but no scalability architecture. |
| separation of model / renderer / controller | Partial | `Graph` + `Renderer` + `GraphLayout`/`GraphNodeCtrl` | Separation exists but states are tightly interwoven. |
| command stack | Absent | No command/transaction types in package | Undoable architecture missing. |
| style/theme support | Partial | Per-node/group/edge colors and widths (`Graph.h`, `GroupNode.h`) | No centralized theming or style system. |
| custom node types | Partial | `Node.shape` enum + manual style fields | No pluggable node-class/render interface. |
| extensible port or edge types | Partial | `Pin.type` int, `PinKind`, edge fields | Type metadata exists, but no registry/validation framework. |
| reusable base classes | Present | `Layout` base class; templated `GraphLayout<T>` | Useful for layout strategy reuse. |
| package boundaries and intended extension points | Partial | `GraphLib.upp`, `Layout` subclasses, `GraphLayout<T>` typedefs | Some extension seams exist, but renderer/controller customization hooks are limited and inconsistent. |

## 4. API and Design Notes
Core abstractions:
- Data model: `Graph`, `Node`, `Edge`, `Pin`, `GroupNode`.
- Layout strategy: `Layout` base + `Spring`, `OrderedTree`, `TournamentTree`.
- Visualization: `Renderer` converts graph state to drawing primitives.
- UI controls:
  - `GraphLayout<T>` is a graph viewer/editor-ish control that couples model + layout + renderer + zoom/scroll.
  - `GraphNodeCtrl` embeds `GraphLayout<Spring>` and adds direct manipulation behaviors.

How tutorials map to implementation:
- Tutorials 1–4 map to `GraphLayout<T>` path, not `GraphNodeCtrl`.
- They validate core model APIs and layout algorithms (`AddNode`, `AddEdge`, shapes, edge labels, Dijkstra highlighting).
- They do not exercise advanced editor concerns (undo stack, robust deserialization, widget embedding, etc.).

Hardcoded vs abstract behavior:
- Hardcoded:
  - Rendering styles and geometry live directly in `Renderer.cpp`.
  - Interaction logic is monolithic in `GraphNodeCtrl.cpp`.
  - File format for saves is hand-rolled text in `SaveGraph`.
- Abstracted:
  - Layout algorithms are swappable via `GraphLayout<T>` and `Layout` subclasses.

Editor-oriented vs viewer-oriented:
- `GraphLayout<T>` is mainly **viewer/layout-oriented** with limited interaction (zoom, route-weight click behavior).
- `GraphNodeCtrl` moves toward **editor-oriented** behavior, but remains incomplete and tightly coupled.
- Net result: more than a drawing helper, but not yet a robust editor framework.

What is missing for a serious node editor:
- Reliable persistence round-trip (load/save schema).
- Undo/redo command architecture.
- Robust edge interaction (selection, reroute, handles).
- Real widget embedding/focus model inside nodes.
- Scalable rendering/hit-test architecture.

## 5. Concrete Gaps
### Foundational gaps
- No full deserialization path (`LoadGraph` is unimplemented).
- Model mixes data, visuals, layout, and transient interaction flags.
- No versioned/documented persistence schema.

### Interaction gaps
- Undo/redo absent.
- Edge selection/rerouting interaction incomplete.
- Group collapse is only a flag toggle, not behavior.
- Internal-only clipboard; no system clipboard integration.
- Pan/zoom behavior split between `GraphLayout` and `GraphNodeCtrl` with unclear single source of truth.

### Layout/routing gaps
- No orthogonal router, no routing policy abstraction.
- No grid snapping or alignment guides.
- No incremental layout/path recompute strategy.
- Tree/DAG layouts are specialized and assumption-heavy, not generalized layered graph layout.

### Architecture gaps
- `GraphNodeCtrl` is large and monolithic.
- Command stack absent; operations mutate model directly.
- Renderer extensibility is limited (few virtual/custom hooks).
- Stale artifacts (`Raphael.h`, `Snap.h`) indicate unfinished cleanup/migration boundaries.

## 6. Recommendations for the Next Planning Task
Suggested implementation tracks:
- **Track A: Core Model & Persistence**
  - Formalize model-vs-view state separation.
  - Define stable graph document schema and implement `LoadGraph` parity with `SaveGraph`.
  - Add migration/version tags.
- **Track B: Editor Interaction & Command System**
  - Introduce command stack (add/remove/move/connect/group/etc.) with undo/redo.
  - Normalize edge selection and manipulation flows.
  - Consolidate pan/zoom state ownership.
- **Track C: Rendering & Performance**
  - Separate scene graph/hit test index from immediate loops.
  - Add culling and dirty-region redraw strategy.
  - Add scalable edge/path cache invalidation.
- **Track D: Layout & Routing**
  - Keep current layouts, add generalized layered layout and orthogonal routing.
  - Add optional snap/guides and routing policy options.
- **Track E: Node UI Embedding**
  - Design host model for per-node child `Ctrl`s and focus routing.
  - Add text editing and interactive widgets inside nodes.

Probable phases and dependencies:
1. **Phase 1 (A)**: persistence + model cleanup first (all later tracks depend on a stable data contract).
2. **Phase 2 (B)**: command architecture next (interaction work should be undoable from the start).
3. **Phase 3 (C + D in parallel)**: performance and routing/layout expansion once operations and model are stable.
4. **Phase 4 (E)**: embedded widgets after command + focus foundations exist.

Capabilities to prioritize first:
- `LoadGraph` and persistence reliability.
- Undo/redo command stack.
- Consistent selection/edge interaction model.
- Pan/zoom and coordinate-system unification.

## 7. Evidence Appendix
- `tutorial/GraphLib1/main.cpp`: tournament tree usage, integer edge API, `SetFillColor`.
- `tutorial/GraphLib2/main.cpp`: ordered dependency graph usage.
- `tutorial/GraphLib3/main.cpp`: node shape/style and directed/labelled edge usage.
- `tutorial/GraphLib4/main.cpp`: weighted edges + Dijkstra route-highlighting workflow.
- `tutorial/GraphLib{1..4}/*.upp`: each tutorial is a GUI example package using `GraphLib`.
- `uppsrc/GraphLib/GraphLib.upp`: package composition; confirms compiled implementation files and excludes stale `Snap.h`/`Raphael.h`.
- `uppsrc/GraphLib/GraphLib.h`: `GraphLayout<T>` control, zoom/scroll, dijkstra interaction, buffer caching and refresh loop.
- `uppsrc/GraphLib/Graph.h`: core model types (`Graph`, `Node`, `Edge`, `Pin`) and mixed logical/visual state.
- `uppsrc/GraphLib/Graph.cpp`: model CRUD and connection/group operations.
- `uppsrc/GraphLib/Renderer.h/.cpp`: rendering and hit-testing implementation, curved edge routing, pin rendering.
- `uppsrc/GraphLib/GroupNode.h`: group/container model and selection/collapse fields.
- `uppsrc/GraphLib/GraphNodeCtrl.h/.cpp`: editor interaction layer (selection, drag, context menus, copy/paste, save/load stubs, animation hooks).
- `uppsrc/GraphLib/Layout.h/.cpp`: base layout abstraction; base `LayoutPrepare` throws not implemented.
- `uppsrc/GraphLib/Spring.*`: force-directed layout algorithm.
- `uppsrc/GraphLib/OrderedTree.*`: topo-sort-based ordered tree layout.
- `uppsrc/GraphLib/TournamentTree.*`: tournament-style tree layout.
- `uppsrc/GraphLib/TopologicalSort.*`: support algorithm for ordered layouts.
- `uppsrc/GraphLib/Dijkstra.*`, `BinaryHeapMap.*`: shortest-path algorithm and heap.
- `uppsrc/GraphLib/Raphael.h`, `Snap.h`: stale/legacy non-C++ pseudo-code artifacts; not part of active package file list.
