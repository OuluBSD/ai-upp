#include "VisualStateModel.h"

namespace Upp {

void VsmRegionAnnotation::Jsonize(JsonIO& json)
{
	json("id",               id)
	    ("parent_id",        parent_id)
	    ("name",             name)
	    ("x",x)("y",y)("w",w)("h",h)
	    ("relative_to_parent", relative_to_parent)
	    ("binding",          binding)
	    ("anchors",          anchors)
	    ("hotspots",         hotspots)
	    ("linked_region_ids",linked_region_ids)
	    ("linked_fingerprints", linked_fingerprints);
}

void VsmAnnotationLayer::Jsonize(JsonIO& json)
{
	json("schema",     schema)
	    ("session_id", session_id)
	    ("annotations",annotations);
}

Vector<VsmAnnotationLayer::ValidationError> VsmAnnotationLayer::Validate() const
{
	Vector<ValidationError> errs;

	// Build id set
	Index<String> id_set;
	for(const VsmRegionAnnotation& a : annotations)
		id_set.Add(a.id);

	// Check for missing parents and cycles
	for(const VsmRegionAnnotation& a : annotations) {
		if(a.id.IsEmpty()) {
			ValidationError& e = errs.Add();
			e.annotation_id = "?";
			e.message = "Annotation has empty id";
		}
		if(!a.parent_id.IsEmpty() && id_set.Find(a.parent_id) < 0) {
			ValidationError& e = errs.Add();
			e.annotation_id = a.id;
			e.message = "Parent not found: " + a.parent_id;
		}
		if(a.w <= 0 || a.h <= 0) {
			ValidationError& e = errs.Add();
			e.annotation_id = a.id;
			e.message = "Invalid rect (zero or negative size)";
		}
	}

	// Cycle detection: DFS from each node
	for(const VsmRegionAnnotation& a : annotations) {
		String cur = a.parent_id;
		int depth  = 0;
		while(!cur.IsEmpty() && depth < (int)annotations.GetCount() + 1) {
			if(cur == a.id) {
				ValidationError& e = errs.Add();
				e.annotation_id = a.id;
				e.message = "Hierarchy cycle detected";
				break;
			}
			const VsmRegionAnnotation* p = FindById(cur);
			cur = p ? p->parent_id : String();
			depth++;
		}
	}
	return errs;
}

Vector<VsmAnnotationLayer::ValidationError> VsmAnnotationLayer::ValidateBounds(int frame_w, int frame_h) const
{
	Vector<ValidationError> errs;

	for(const VsmRegionAnnotation& a : annotations) {
		// Check if annotation rectangle is fully within [0, 0, frame_w, frame_h]
		if(a.x < 0 || a.y < 0 || a.x + a.w > frame_w || a.y + a.h > frame_h) {
			ValidationError& e = errs.Add();
			e.annotation_id = a.id;
			e.message = Format("Annotation rect [%d,%d,%d,%d] outside bounds [0,0,%d,%d]",
			                  a.x, a.y, a.x + a.w, a.y + a.h, frame_w, frame_h);
		}
	}

	return errs;
}

const VsmRegionAnnotation* VsmAnnotationLayer::FindById(const VsmAnnotationId& id) const
{
	for(const VsmRegionAnnotation& a : annotations)
		if(a.id == id) return &a;
	return nullptr;
}

VsmRegionAnnotation* VsmAnnotationLayer::FindById(const VsmAnnotationId& id)
{
	for(VsmRegionAnnotation& a : annotations)
		if(a.id == id) return &a;
	return nullptr;
}

bool VsmAnnotationLayer::Save(const String& path) const
{
	String json = StoreAsJson(*this, true);
	return SaveFile(path, json);
}

bool VsmAnnotationLayer::Load(const String& path)
{
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	return LoadFromJson(*this, json);
}

} // namespace Upp
