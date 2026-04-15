#ifndef _Node_Core_Editor_h_
#define _Node_Core_Editor_h_

#include "Scene.h"

namespace Upp {

namespace Node {

enum class EditorMode {
	READY,
	MARQUEE,
	DRAGGING,
	LINKING
};

struct EditorState : public Moveable<EditorState> {
	EditorMode mode = EditorMode::READY;
	
	Index<EntityId> selection;
	EntityId        hovered_entity;
	SceneItem::Type hovered_type;
	
	EntityId        focused_widget; // EntityId of the slot that has focus
	
	Pointf          mouse_pos;
	Pointf          drag_start;
	Rectf           marquee_rect;
	
	// Linking state
	EntityId        link_source_node;
	EntityId        link_source_pin;
	
	void ClearSelection() { selection.Clear(); }
	void Select(const EntityId& id) { if(selection.Find(id) < 0) selection.Add(id); }
	void Deselect(const EntityId& id) { int i = selection.Find(id); if(i >= 0) selection.Remove(i); }
	bool IsSelected(const EntityId& id) const { return selection.Find(id) >= 0; }
};

} // namespace Node

} // namespace Upp

#endif
