#ifndef _AI_CodeVisitor_h_
#define _AI_CodeVisitor_h_

NAMESPACE_UPP

struct CodeVisitor
{
	struct Item : Moveable<Item> {
		const AnnotationItem* ann = 0;
		const ReferenceItem* ref = 0;
		String error_id;
		Point pos;
		bool operator()(const Item& a, const Item& b) const {return a.pos.y != b.pos.y ? a.pos.y < b.pos.y : a.pos.x < b.pos.x;}
		String ToString() const {return ann ? ann->ToString() : (ref ? ref->ToString() : (error_id.GetCount() ? "error(" + error_id + ")" : "<empty>"));}
	};
	int limit = 0;
	Vector<Item> items;
	Vector<Item> export_items;
	VectorMap<String,String> files;
	Index<String> visited_refs;
	Index<String> visited_defs;
	Index<String> visited_decls;
	
	CodeVisitor() {}
	void SetLimit(int i) {limit = i;}
	void Begin();
	void Visit(const String& filepath, const FileAnnotation& fa, Point begin, Point end);
	void Visit(const String& filepath, const AnnotationItem& ann);
	void Visit(const String& filepath, const ReferenceItem& ref);
	void VisitId(const String& filepath, const String& id);
	const String& LoadPath(String path);
};

END_UPP_NAMESPACE

#endif
