#ifndef _Node_Ctrl_Ctrl_h_
#define _Node_Ctrl_Ctrl_h_

#include <CtrlLib/CtrlLib.h>
#include <Node/Core/Core.h>
#include <Node/Core/Scene.h>
#include <Node/Core/Editor.h>
#include <Node/Core/Command.h>
#include <Node/Core/Snap.h>
#include <Node/Core/Viewport.h>

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
	bool                 vp_pristine = true;  // true until user first pans/zooms
	double               anim_phase = 0.0;          // for Realistic edge animation [0..1)

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
	
	void ZoomToFit();
	void SetEdgeStyle(EdgeStyle s);
	void SetAnimPhase(double phase);  // called by owner for Realistic animation
	// Register a node type for the "Add Node" menu. factory() returns a pre-filled template.
	void RegisterNodeType(const String& type_id, const String& label,
	                      Function<NodeDoc()> factory);
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
