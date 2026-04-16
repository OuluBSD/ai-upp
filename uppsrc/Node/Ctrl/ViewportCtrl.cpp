#include "Ctrl.h"
#include <Node/Core/Clipboard.h>

namespace Upp {

namespace Node {

NodeViewportCtrl::NodeViewportCtrl()
{
	BackPaint();
}

static Ctrl* CreateSlotWidget(const String& widget_type, const String& entity_id,
                               NodeViewportCtrl* viewport,
                               Graph* graph, EditorState* editor,
                               HistoryStack* history, CommandDispatcher* dispatcher)
{
	// Registered factory first
	// (caller checks factories before calling this)

	if(widget_type == "Button") {
		Button* b = new Button();
		b->SetLabel("OK");
		return b;
	}
	if(widget_type == "EditField" || widget_type == "EditString" || widget_type == "param") {
		EditField* ef = new EditField();
		String eid = entity_id;
		ef->WhenAction = [viewport, ef, eid, graph, editor, history, dispatcher] {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("value", ef->GetData());
			history->Execute(CommandContext(*graph, *editor), dispatcher->Create("SetWidgetValue", arg));
		};
		return ef;
	}
	if(widget_type == "EditIntSpin") {
		EditInt* ei = new EditInt();
		String eid = entity_id;
		ei->WhenAction = [viewport, ei, eid, graph, editor, history, dispatcher] {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("value", ei->GetData());
			history->Execute(CommandContext(*graph, *editor), dispatcher->Create("SetWidgetValue", arg));
		};
		return ei;
	}
	if(widget_type == "EditDoubleSpin") {
		EditDouble* ed = new EditDouble();
		String eid = entity_id;
		ed->WhenAction = [viewport, ed, eid, graph, editor, history, dispatcher] {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("value", ed->GetData());
			history->Execute(CommandContext(*graph, *editor), dispatcher->Create("SetWidgetValue", arg));
		};
		return ed;
	}
	if(widget_type == "DropList") {
		DropList* dl = new DropList();
		return dl;
	}
	if(widget_type == "Option") {
		Option* opt = new Option();
		opt->SetLabel("");
		String eid = entity_id;
		opt->WhenAction = [viewport, opt, eid, graph, editor, history, dispatcher] {
			ValueMap arg;
			arg.Add("id", eid);
			arg.Add("value", opt->GetData());
			history->Execute(CommandContext(*graph, *editor), dispatcher->Create("SetWidgetValue", arg));
		};
		return opt;
	}
	if(widget_type == "DocEdit") {
		DocEdit* de = new DocEdit();
		return de;
	}
	// Fallback: read-only label
	StaticText* st = new StaticText();
	return st;
}

void NodeViewportCtrl::SyncWidgets()
{
	Index<EntityId> current_ids;
	EntityId new_focused;

	for(const auto& item : scene.items) {
		if(item.type != SceneItem::WIDGET) continue;
		if(!item.image_path.IsEmpty()) continue; // image widgets painted directly, no Ctrl

		current_ids.Add(item.entity_id);
		int q = host.widgets.Find(item.entity_id);
		Ctrl* w = nullptr;
		if(q < 0) {
			// Registered factory first
			int fi = widget_factories.Find(item.text);
			if(fi >= 0) {
				w = widget_factories[fi]();
			} else {
				w = CreateSlotWidget(item.text, item.entity_id,
				                     this, graph, editor, history, dispatcher);
			}
			host.widgets.Add(item.entity_id, w);
			Add(*w);
		} else {
			w = &host.widgets[q];
		}

		// Update value from doc if not focused
		if(!w->HasFocus()) {
			int sep = item.entity_id.Find(':');
			if(sep >= 0) {
				EntityId node_id = item.entity_id.Left(sep);
				EntityId slot_id = item.entity_id.Mid(sep + 1);
				NodeDoc* n = graph->FindNode(node_id);
				if(n) {
					for(int si = 0; si < n->slots.GetCount(); si++) {
						WidgetSlotDoc& s = n->slots[si];
						if(s.id == slot_id) {
							Value v = s.properties.Get("value", Value());
							if(!v.IsVoid()) {
								// Coerce value to the type expected by each widget
								// to avoid ValueTypeError inside SetData
								if(s.type == "EditDoubleSpin") {
									double d = 0;
									if(IsNumber(v))       d = double(v);
									else if(IsString(v))  d = ScanDouble(v.ToString());
									w->SetData(d);
								} else if(s.type == "EditIntSpin") {
									int64 i = 0;
									if(IsNumber(v))       i = int64(v);
									else if(IsString(v))  i = ScanInt64(v.ToString());
									w->SetData(i);
								} else if(s.type == "Option") {
									bool b = false;
									if(IsNumber(v))       b = (int(v) != 0);
									else if(IsString(v))  b = (v.ToString() == "true" ||
									                           v.ToString() == "1");
									w->SetData((int)b);
								} else {
									// EditField, DropList, DocEdit, StaticText — accept string
									w->SetData(v.ToString());
								}
							}
							break;
						}
					}
				}
			}
		}

		if(w->HasFocus())
			new_focused = item.entity_id;

		Rect r = vp.WorldToView(item.rect);
		if(w->GetRect() != r)
			w->SetRect(r);
	}

	if(editor) editor->focused_widget = new_focused;

	// Remove stale widgets
	for(int i = host.widgets.GetCount() - 1; i >= 0; i--) {
		if(current_ids.Find(host.widgets.GetKey(i)) < 0) {
			host.widgets[i].Remove();
			host.widgets.Remove(i);
		}
	}
}

void NodeViewportCtrl::ZoomToFit()
{
	if(graph) {
		if(builder.IsDirty(*graph))
			builder.Build(scene, *graph);
		vp.ZoomToFit(scene.index.bounds, GetSize());
		Refresh();
	}
}

void NodeViewportCtrl::Paint(Draw& w)
{
	Size sz = GetSize();
	PaintBackground(w, sz, vp);

	if(graph) {
		if(builder.IsDirty(*graph)) {
			builder.Build(scene, *graph);
		}

		if(fit_on_first_paint && !scene.items.IsEmpty() && sz.cx > 100 && sz.cy > 100) {
			fit_on_first_paint = false;
			vp.ZoomToFit(scene.index.bounds, sz);
		}

		PaintScene(w, scene, vp, editor);
		
		if(editor && editor->mode == EditorMode::MARQUEE) {
			Rect r = vp.WorldToView(editor->marquee_rect);
			DrawFrame(w, r, Cyan());
			// Transparent Rect not directly supported by Draw, using thin frame or just solid for now
			// w.DrawRect(r, Cyan()); 
		}
		
		SyncWidgets();
	}
}

void NodeViewportCtrl::MouseWheel(Point p, int zdelta, dword key)
{
	double factor = (zdelta > 0 ? 1.1 : 0.9);
	vp.Zoom(vp.GetScale() * factor, p);
	Refresh();
}

void NodeViewportCtrl::LeftDown(Point p, dword key)
{
	SetFocus();
	SetCapture();
	last_mouse_pos = p;
	drag_pending = false;

	if(editor && dispatcher && history) {
		if(builder.IsDirty(*graph))
			builder.Build(scene, *graph);

		Scene::HitResult hit = scene.HitTest(vp.ViewToWorld(p));

		if(hit) {
			if(hit.type == SceneItem::PIN) {
				editor->mode = EditorMode::LINKING;
				int sep = hit.entity_id.Find(':');
				if(sep >= 0) {
					editor->link_source_node = hit.entity_id.Left(sep);
					editor->link_source_pin = hit.entity_id.Mid(sep + 1);
				}
			}
			else {
				// A LABEL hit on a plain node label (no ':' in id) is treated as a NODE hit
				// so that clicking the title bar starts a drag.
				SceneItem::Type effective_type = hit.type;
				if(hit.type == SceneItem::LABEL && hit.entity_id.Find(':') < 0) {
					if(graph->FindNode(hit.entity_id))
						effective_type = SceneItem::NODE;
				}

				ValueMap arg;
				arg.Add("id", hit.entity_id);
				arg.Add("exclusive", !(key & K_CTRL));
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("Select", arg));

				if(effective_type == SceneItem::NODE) {
					editor->mode = EditorMode::DRAGGING;
					editor->drag_start = vp.ViewToWorld(p);
					drag_start_view = p;
					drag_pending = true;
					history->Begin();
					WhenNodeClick(hit.entity_id);
				}
				else if(effective_type == SceneItem::EDGE) {
					WhenEdgeClick(hit.entity_id);
				}
			}
		}
		else {
			editor->mode = EditorMode::MARQUEE;
			editor->drag_start = vp.ViewToWorld(p);
			editor->marquee_rect = Rectf(editor->drag_start, Sizef(0, 0));
			if(!(key & K_CTRL))
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("ClearSelection", ValueMap()));
		}
		Refresh();
	}
}

void NodeViewportCtrl::MouseMove(Point p, dword key)
{
	// Pan via middle mouse
	if(panning) {
		vp.Pan(Pointf(p.x - last_mouse_pos.x, p.y - last_mouse_pos.y));
		last_mouse_pos = p;
		Refresh();
		return;
	}

	/* OS Drag-and-drop temporarily disabled as it interferes with internal node movement
	if(drag_pending && editor && graph && editor->selection.GetCount() &&
	   (abs(p.x - drag_start_view.x) > 4 || abs(p.y - drag_start_view.y) > 4)) {
		drag_pending = false;
		String payload = StoreClipboard(*graph, editor->selection);
		VectorMap<String, ClipData> data;
		Append(data, payload);
		DoDragAndDrop(data, Null, DND_COPY | DND_MOVE);
		// DoDragAndDrop is blocking; clean up any pending transaction
		history->Commit();
		editor->mode = EditorMode::READY;
		Refresh();
		return;
	}
	*/

	if(editor && (key & K_MOUSELEFT)) {
		if(editor->mode == EditorMode::DRAGGING) {
			Pointf delta = vp.ViewToWorld(p) - vp.ViewToWorld(last_mouse_pos);
			for(const auto& id : editor->selection) {
				NodeDoc* n = graph->FindNode(id);
				if(n) {
					Pointf new_pos = n->pos + delta;
					SnapRequest sreq;
					sreq.point    = new_pos;
					sreq.grid_step = 10.0;
					sreq.tolerance = 5.0;
					new_pos = SnapToGrid(sreq).snapped_point;
					ValueMap arg;
					arg.Add("id", id);
					arg.Add("x", new_pos.x);
					arg.Add("y", new_pos.y);
					history->Execute(CommandContext(*graph, *editor), dispatcher->Create("MoveNode", arg));
				}
			}
		}
		else if(editor->mode == EditorMode::MARQUEE) {
			editor->marquee_rect = Rectf(editor->drag_start, vp.ViewToWorld(p)).Normalized();
		}
		Refresh();
	}

	// Hover tracking — update hovered entity and refresh if changed
	if(editor && graph) {
		Scene::HitResult hit = scene.HitTest(vp.ViewToWorld(p));
		EntityId new_hover = hit ? hit.entity_id : EntityId();
		if(new_hover != editor->hovered_entity) {
			editor->hovered_entity = new_hover;
			editor->hovered_type   = hit ? hit.type : SceneItem::NODE;
			Refresh();
		}
	}

	last_mouse_pos = p;
}

void NodeViewportCtrl::MiddleDown(Point p, dword key)
{
	SetFocus();
	panning = true;
	last_mouse_pos = p;
	SetCapture();
}

void NodeViewportCtrl::MiddleUp(Point p, dword key)
{
	panning = false;
	ReleaseCapture();
}

void NodeViewportCtrl::LeftUp(Point p, dword key)
{
	ReleaseCapture();
	if(editor) {
		if(editor->mode == EditorMode::DRAGGING) {
			history->Commit();
		}
		else if(editor->mode == EditorMode::MARQUEE) {
			Vector<EntityId> ids = scene.MarqueeSelect(editor->marquee_rect);
			for(const auto& id : ids) {
				ValueMap arg;
				arg.Add("id", id);
				arg.Add("exclusive", false);
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("Select", arg));
			}
		}
		else if(editor->mode == EditorMode::LINKING) {
			Scene::HitResult hit = scene.HitTest(vp.ViewToWorld(p));
			if(hit && hit.type == SceneItem::PIN) {
				int sep = hit.entity_id.Find(':');
				if(sep >= 0) {
					EntityId target_node = hit.entity_id.Left(sep);
					EntityId target_pin = hit.entity_id.Mid(sep + 1);
					
					if(target_node != editor->link_source_node) {
						ValueMap arg;
						arg.Add("id", "e_" + Uuid::Create().ToString());
						arg.Add("source_node", editor->link_source_node);
						arg.Add("source_pin", editor->link_source_pin);
						arg.Add("target_node", target_node);
						arg.Add("target_pin", target_pin);
						history->Execute(CommandContext(*graph, *editor), dispatcher->Create("AddEdge", arg));
					}
				}
			}
		}
		editor->mode = EditorMode::READY;
		Refresh();
	}
}

void NodeViewportCtrl::RightDown(Point p, dword key)
{
	SetFocus();
	if(editor && graph && dispatcher && history) {
		Pointf wp = vp.ViewToWorld(p);
		Scene::HitResult hit = scene.HitTest(wp);
		MenuBar menu;

		if(hit) {
			if(hit.type == SceneItem::NODE) {
				menu.Add("Remove Node", [=] {
					ValueMap arg; arg.Add("id", hit.entity_id);
					history->Execute(CommandContext(*graph, *editor), dispatcher->Create("RemoveNode", arg));
					Refresh();
				});
				if(editor->IsSelected(hit.entity_id) && editor->selection.GetCount() > 1) {
					menu.Add("Remove Selected Nodes", [=] {
						history->Begin();
						for(const auto& id : editor->selection) {
							ValueMap arg; arg.Add("id", id);
							history->Execute(CommandContext(*graph, *editor), dispatcher->Create("RemoveNode", arg));
						}
						history->Commit();
						Refresh();
					});
				}
			}
			else if(hit.type == SceneItem::EDGE) {
				menu.Add("Remove Edge", [=] {
					// RemoveEdge via direct graph mutation + invalidate (no command yet — use graph directly)
					graph->RemoveEdge(hit.entity_id);
					Refresh();
				});
			}
			else if(hit.type == SceneItem::GROUP) {
				menu.Add("Remove Group", [=] {
					graph->RemoveGroup(hit.entity_id);
					Refresh();
				});
			}
			menu.Separator();
			menu.Add("Select All", [=] {
				history->Begin();
				for(const auto& n : graph->GetDoc().nodes) {
					ValueMap arg; arg.Add("id", n.id); arg.Add("exclusive", false);
					history->Execute(CommandContext(*graph, *editor), dispatcher->Create("Select", arg));
				}
				history->Commit();
				Refresh();
			});
		}
		else {
			menu.Add("Add Node", [=] {
				ValueMap arg;
				arg.Add("id", "n_" + Uuid::Create().ToString());
				arg.Add("x", wp.x);
				arg.Add("y", wp.y);
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("AddNode", arg));
				Refresh();
			});
			String clip = ReadClipboardText();
			if(!clip.IsEmpty()) {
				menu.Add("Paste", [=] {
					Vector<ValidationMessage> errors;
					history->Begin();
					if(LoadClipboard(*graph, clip, wp, errors))
						history->Commit();
					else
						history->Abort(CommandContext(*graph, *editor));
					Refresh();
				});
			}
			menu.Separator();
			menu.Add("Select All", [=] {
				history->Begin();
				for(const auto& n : graph->GetDoc().nodes) {
					ValueMap arg; arg.Add("id", n.id); arg.Add("exclusive", false);
					history->Execute(CommandContext(*graph, *editor), dispatcher->Create("Select", arg));
				}
				history->Commit();
				Refresh();
			});
			menu.Add("Clear Selection", [=] {
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("ClearSelection", ValueMap()));
				Refresh();
			});
		}

		menu.Execute();
	}
}

bool NodeViewportCtrl::Key(dword key, int count)
{
	if(editor && !editor->focused_widget.IsEmpty())
		return false;
	
	if(history && editor && dispatcher) {
		if(key == K_CTRL_C) {
			if(editor->selection.GetCount()) {
				ClearClipboard();
				AppendClipboardText(StoreClipboard(*graph, editor->selection));
			}
			return true;
		}
		if(key == K_CTRL_V) {
			String data = ReadClipboardText();
			if(!data.IsEmpty()) {
				Vector<ValidationMessage> errors;
				history->Begin();
				if(LoadClipboard(*graph, data, vp.ViewToWorld(last_mouse_pos), errors)) {
					history->Commit();
					Refresh();
				}
				else {
					history->Abort(CommandContext(*graph, *editor));
				}
			}
			return true;
		}
		if(key == K_CTRL_Z) {
			history->Undo(CommandContext(*graph, *editor));
			Refresh();
			return true;
		}
		if(key == K_CTRL_Y) {
			history->Redo(CommandContext(*graph, *editor));
			Refresh();
			return true;
		}
		if(key == K_DELETE) {
			history->Begin();
			for(const auto& id : editor->selection) {
				ValueMap arg;
				arg.Add("id", id);
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("RemoveNode", arg));
			}
			history->Commit();
			Refresh();
			return true;
		}
		if(key == K_F) {
			ZoomToFit();
			return true;
		}
	}
	return false;
}

void NodeViewportCtrl::DragAndDrop(Point p, PasteClip& d)
{
	if(!graph || !history || !editor || !dispatcher) return;

	// Accept text/plain drops (our own JSON subgraph payload or external text)
	if(d.IsAnyAvailable(ClipFmtsText())) {
		if(d.Accept()) {
			String data = GetString(d);
			if(!data.IsEmpty()) {
				Vector<ValidationMessage> errors;
				Pointf wp = vp.ViewToWorld(p);
				history->Begin();
				if(LoadClipboard(*graph, data, wp, errors))
					history->Commit();
				else
					history->Abort(CommandContext(*graph, *editor));
				Refresh();
			}
		}
	}
}

} // namespace Node

} // namespace Upp
