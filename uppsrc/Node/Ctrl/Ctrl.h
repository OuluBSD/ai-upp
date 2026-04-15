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
	bool                 panning = false;  // middle-mouse or space+drag pan
	bool                 drag_pending = false; // left-drag threshold not yet crossed
	Point                drag_start_view; // view-space origin for DnD threshold
	
	typedef Function<Ctrl*()> WidgetFactory;
	VectorMap<String, WidgetFactory> widget_factories;

	struct WidgetHost : public Moveable<WidgetHost> {
		ArrayMap<EntityId, Ctrl> widgets;
	} host;
	
	void SyncWidgets();
	
public:
	typedef NodeViewportCtrl CLASSNAME;
	
	NodeViewportCtrl();
	
	void SetGraph(Graph& g) { graph = &g; Refresh(); }
	void SetEditor(EditorState& e) { editor = &e; Refresh(); }
	void SetHistory(HistoryStack& h) { history = &h; }
	void SetDispatcher(CommandDispatcher& d) { dispatcher = &d; }
	void RegisterWidget(const String& type, WidgetFactory f) { widget_factories.Add(type, f); }
	
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
};

} // namespace Node

} // namespace Upp

#endif
