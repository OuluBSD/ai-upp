#ifndef _Node_Ctrl_Ctrl_h_
#define _Node_Ctrl_Ctrl_h_

#include <CtrlLib/CtrlLib.h>
#include <Node/Core/Core.h>
#include <Node/Core/Scene.h>
#include <Node/Core/Editor.h>
#include <Node/Core/Command.h>
#include <Node/Core/Snap.h>
#include <Node/Core/Viewport.h>
#include <Node/Core/Layout.h>

namespace Upp {

namespace Node {

void PaintBackground(Draw& w, Size sz, const Viewport& vp);
void PaintScene(Draw& w, const Scene& scene, const Viewport& vp, const EditorState* es = nullptr);

class NodeViewportCtrl : public Ctrl {
	Graph*             graph = nullptr;
	EditorState*       editor = nullptr;
	HistoryStack*      history = nullptr;
	CommandDispatcher* dispatcher = nullptr;
	
	Viewport             vp;
	Scene                scene;
	BaselineSceneBuilder builder;
	Point                last_mouse_pos;
	bool                 panning = false;         // middle-mouse pan in progress
	bool                 drag_pending = false;    // left-drag DnD threshold not yet crossed
	Point                drag_start_view;         // view-space origin for DnD threshold
	bool                 fit_on_first_paint = false; // ZoomToFit on next Paint after SetGraph
	bool                 vp_pristine = true;        // true until user first pans/zooms
	bool                 auto_layout_active = false; // true while automatic layout is in effect; manual drag clears it
	double               anim_phase = 0.0;          // for Realistic edge animation [0..1)
	SmartPacker::LayoutOrientation layout_orientation = SmartPacker::LAYOUT_TALL;
	bool force_refine = true;  // run ForceRefine after any layout

	// Custom layout registry: name → callback
	struct CustomLayoutEntry : Moveable<CustomLayoutEntry> {
		String name;
		Function<void(Graph&)> fn;
	};
	Vector<CustomLayoutEntry> custom_layouts;
	String active_custom_layout;  // empty = using built-in orientation

	// Node type registry: type_id -> template NodeDoc factory
	struct NodeTypeEntry : Moveable<NodeTypeEntry> {
		String        type_id;    // e.g. "comfyui.gguf.unet_loader"
		String        category;   // first segment
		String        label;      // display name
		Function<NodeDoc()> factory; // returns a pre-filled NodeDoc template
	};
	Vector<NodeTypeEntry> node_type_registry;

	typedef Function<Ctrl*()> WidgetFactory;
	VectorMap<String, WidgetFactory> widget_factories;

	struct WidgetHost : public Moveable<WidgetHost> {
		ArrayMap<EntityId, Ctrl> widgets;
	} host;
	
	void SyncWidgets();
	
public:
	typedef NodeViewportCtrl CLASSNAME;
	
	NodeViewportCtrl();
	
	// Callbacks — fired after the default handler, before Refresh()
	Event<EntityId> WhenNodeClick;  // left-click on a node (entity id)
	Event<EntityId> WhenEdgeClick;  // left-click on an edge

	void SetGraph(Graph& g) { graph = &g; fit_on_first_paint = true; vp_pristine = true; Refresh(); }
	void SetEditor(EditorState& e) { editor = &e; Refresh(); }
	void SetHistory(HistoryStack& h) { history = &h; }
	void SetDispatcher(CommandDispatcher& d) { dispatcher = &d; }
	void RegisterWidget(const String& type, WidgetFactory f) { widget_factories.Add(type, f); }
	
	void             ZoomToFit();
	void             SetEdgeStyle(EdgeStyle s);
	void             SetAnimPhase(double phase);  // called by owner for Realistic animation
	// Register a node type for the "Add Node" menu. factory() returns a pre-filled template.
	void             RegisterNodeType(const String& type_id, const String& label,
	                                  Function<NodeDoc()> factory);
	
	// Pan/zoom accessors
	Pointf  GetPanOffset() const   { return vp.GetOffset(); }
	double  GetZoomScale() const   { return vp.GetScale(); }
	void    SetPanZoom(Pointf offset, double scale) { vp.SetOffset(offset); vp.SetScale(scale); vp_pristine = false; Refresh(); }

	// Layout orientation
	SmartPacker::LayoutOrientation GetLayoutOrientation() const { return layout_orientation; }
	void             SetLayoutOrientation(SmartPacker::LayoutOrientation o) { layout_orientation = o; active_custom_layout = String(); }
	void             ToggleLayoutOrientation() { layout_orientation = (layout_orientation == SmartPacker::LAYOUT_TALL) ? SmartPacker::LAYOUT_WIDE : SmartPacker::LAYOUT_TALL; active_custom_layout = String(); }
	bool             GetForceRefine() const  { return force_refine; }
	void             SetForceRefine(bool b)  { force_refine = b; }

	// Register a named custom layout function (appears in the Layout Orientation menu).
	void             RegisterLayout(const String& name, Function<void(Graph&)> fn);
	// Activate a previously registered layout by name (clears built-in orientation selection).
	void             SetActiveLayout(const String& name) { active_custom_layout = name; layout_orientation = SmartPacker::LAYOUT_TALL; }

	// Edit actions (for menu/toolbar use)
	bool CanUndo() const { return history && history->CanUndo(); }
	bool CanRedo() const { return history && history->CanRedo(); }
	bool HasSelection() const { return editor && editor->selection.GetCount() > 0; }
	void DoUndo() { if(history && graph && editor) { history->Undo(CommandContext(*graph, *editor)); Refresh(); } }
	void DoRedo() { if(history && graph && editor) { history->Redo(CommandContext(*graph, *editor)); Refresh(); } }
	void DoDeleteSelection();
	void DoSelectAll();

	void             ApplyLayout();  // Re-run layout with current orientation
	virtual void Layout() override; // handles auto-refit on size change
	virtual void Paint(Draw& w) override;
	virtual void MouseWheel(Point p, int zdelta, dword key) override;
	virtual void LeftDown(Point p, dword key) override;
	virtual void MouseMove(Point p, dword key) override;
	virtual void LeftUp(Point p, dword key) override;
	virtual void MiddleDown(Point p, dword key) override;
	virtual void MiddleUp(Point p, dword key) override;
	virtual void RightDown(Point p, dword key) override;
	virtual bool Key(dword key, int count) override;
	virtual void DragAndDrop(Point p, PasteClip& d) override;
	virtual Image CursorImage(Point p, dword key) override;
};

} // namespace Node

} // namespace Upp

#endif
