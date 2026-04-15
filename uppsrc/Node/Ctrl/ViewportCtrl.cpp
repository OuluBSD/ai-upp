#include "Ctrl.h"
#include <Node/Core/Clipboard.h>

namespace Upp {

namespace Node {

NodeViewportCtrl::NodeViewportCtrl()
{
	BackPaint();
}

void NodeViewportCtrl::SyncWidgets()
{
	Index<EntityId> current_ids;
	EntityId new_focused;
	
	for(const auto& item : scene.items) {
		if(item.type == SceneItem::WIDGET) {
			current_ids.Add(item.entity_id);
			int q = host.widgets.Find(item.entity_id);
			Ctrl* w = nullptr;
			if(q < 0) {
				// Check registered factory first, then built-in fallbacks
				int fi = widget_factories.Find(item.text);
				if(fi >= 0) {
					w = widget_factories[fi]();
				}
				else if(item.text == "Button") {
					Button* b = new Button();
					b->SetLabel("OK");
					w = b;
				}
				else if(item.text == "EditField") {
					EditField* ef = new EditField();
					ef->WhenAction = [=] {
						ValueMap arg;
						arg.Add("id", item.entity_id);
						arg.Add("value", ef->GetData());
						history->Execute(CommandContext(*graph, *editor), dispatcher->Create("SetWidgetValue", arg));
					};
					w = ef;
				}
				else {
					w = new StaticText();
				}

				host.widgets.Add(item.entity_id, w);
				Add(*w);
			}
			else {
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
						for(const auto& s : n->slots) {
							if(s.id == slot_id) {
								w->SetData(s.properties.Get("value", Value()));
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

void NodeViewportCtrl::Paint(Draw& w)
{
	Size sz = GetSize();
	w.DrawRect(sz, White());
	
	if(graph) {
		if(builder.IsDirty(*graph)) {
			RTIMESTOP("Scene Build");
			builder.Build(scene, *graph);
			graph->GetMetrics().scene_build_ms = RTIMESTOP_GET("Scene Build");
		}
		
		{
			RTIMESTOP("Paint Bridge");
			PaintScene(w, scene, vp, editor);
			graph->GetMetrics().paint_bridge_ms = RTIMESTOP_GET("Paint Bridge");
		}
		
		if(editor && editor->mode == EditorMode::MARQUEE) {
			Rect r = vp.WorldToView(editor->marquee_rect);
			DrawFrame(w, r, Cyan());
			w.DrawRect(r, Color(0, 255, 255, 64));
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
	last_mouse_pos = p;
	
	if(editor && dispatcher && history) {
		Scene::HitResult hit;
		{
			RTIMESTOP("Hit Test");
			hit = scene.HitTest(vp.ViewToWorld(p));
			graph->GetMetrics().hit_query_ms = RTIMESTOP_GET("Hit Test");
		}
		
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
				ValueMap arg;
				arg.Add("id", hit.entity_id);
				arg.Add("exclusive", !(key & K_CTRL));
				history->Execute(CommandContext(*graph, *editor), dispatcher->Create("Select", arg));
				
				if(hit.type == SceneItem::NODE) {
					editor->mode = EditorMode::DRAGGING;
					editor->drag_start = vp.ViewToWorld(p);
					history->Begin();
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
		vp.Pan(Pointf(p - last_mouse_pos));
		last_mouse_pos = p;
		Refresh();
		return;
	}

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
	}
	return false;
}

} // namespace Node

} // namespace Upp
