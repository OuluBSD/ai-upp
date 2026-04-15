#include "Ctrl.h"

namespace Upp {

namespace Node {

void PaintScene(Draw& w, const Scene& scene, const Viewport& vp, const EditorState* es)
{
	for(const auto& item : scene.items) {
		bool selected = es && es->IsSelected(item.entity_id);
		bool hovered = es && es->hovered_entity == item.entity_id;
		
		Color fill = item.fill_clr;
		Color line = item.line_clr;
		int width = item.line_width;
		
		if(selected) line = LtRed();
		if(hovered) line = Cyan();
		if(selected || hovered) width += 1;

		if(item.type == SceneItem::NODE || item.type == SceneItem::PIN || item.type == SceneItem::GROUP) {
			Rect r = vp.WorldToView(item.rect);
			if(!IsNull(fill))
				w.DrawRect(r, fill);
			if(!IsNull(line))
				DrawFrame(w, r, line);
		}
		else if(item.type == SceneItem::EDGE) {
			if(item.path.GetCount() > 1) {
				Vector<Point> pts;
				for(const auto& p : item.path)
					pts.Add(vp.WorldToView(p));
				w.DrawPolyline(pts, width, line);
			}
		}
		else if(item.type == SceneItem::LABEL) {
			Rect r = vp.WorldToView(item.rect);
			Font f = StdFont();
			if(!item.font_face.IsEmpty()) f.FaceName(item.font_face);
			f.Height((int)(item.font_height * vp.GetScale()));
			if(item.font_bold) f.Bold();
			if(item.font_italic) f.Italic();
			
			w.DrawText(r.left, r.top, item.text, f, line);
		}
	}
}

} // namespace Node

} // namespace Upp
