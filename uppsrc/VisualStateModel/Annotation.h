#ifndef _VisualStateModel_Annotation_h_
#define _VisualStateModel_Annotation_h_

namespace Upp {

typedef String VsmAnnotationId;

struct VsmAnchorPoint : Moveable<VsmAnchorPoint> {
	String name;
	int    x = 0, y = 0; // relative to annotation rect
	void Jsonize(JsonIO& json) { json("name",name)("x",x)("y",y); }
};

struct VsmHotspot : Moveable<VsmHotspot> {
	String name;
	int    x = 0, y = 0;
	void Jsonize(JsonIO& json) { json("name",name)("x",x)("y",y); }
};

// Binding hint: how the annotation moves when its parent or frame moves.
enum VsmRegionBindingType {
	VSM_BIND_FIXED      = 0,
	VSM_BIND_HORIZONTAL = 1,
	VSM_BIND_VERTICAL   = 2,
	VSM_BIND_REFERENCE  = 3,
};

struct VsmRegionBinding : Moveable<VsmRegionBinding> {
	int    type = VSM_BIND_FIXED; // VsmRegionBindingType as int
	String reference_id;          // for REFERENCE binding
	void Jsonize(JsonIO& json) { json("type",type)("reference_id",reference_id); }
};

struct VsmRegionAnnotation : Moveable<VsmRegionAnnotation> {
	VsmAnnotationId id, parent_id;
	String          name;
	int             x=0, y=0, w=0, h=0;
	bool            relative_to_parent = false;
	VsmRegionBinding binding;
	Vector<VsmAnchorPoint> anchors;
	Vector<VsmHotspot>     hotspots;
	// Link to observed regions / fingerprints
	Vector<String>  linked_region_ids;
	Vector<String>  linked_fingerprints;

	void Jsonize(JsonIO& json);
};

struct VsmAnnotationLayer : Moveable<VsmAnnotationLayer> {
	int    schema     = 1;
	String session_id;
	Vector<VsmRegionAnnotation> annotations;

	void Jsonize(JsonIO& json);

	// Validation
	struct ValidationError : Moveable<ValidationError> {
		VsmAnnotationId annotation_id;
		String          message;
	};

	Vector<ValidationError> Validate() const;
	Vector<ValidationError> ValidateBounds(int frame_w, int frame_h) const;

	// Helpers
	const VsmRegionAnnotation* FindById(const VsmAnnotationId& id) const;
	VsmRegionAnnotation*       FindById(const VsmAnnotationId& id);

	bool Save(const String& path) const;
	bool Load(const String& path);
};

} // namespace Upp

#endif
