#include "AI.h"

NAMESPACE_UPP

bool CodeVisitor::Item::operator()(const Item& a, const Item& b) const
{
	if (a.file != b.file) return a.file < b.file;
	if (a.pos.y != b.pos.y) return a.pos.y < b.pos.y;
	return a.pos.x < b.pos.x;
}
String CodeVisitor::Item::ToString() const
{
	return ann ? ann->ToString()
	           : (ref ? ref->ToString()
	                  : (error_id.GetCount() ? "error(" + error_id + ")" : "<empty>"));
}

void CodeVisitor::Begin()
{
	items.Clear();
	export_items.Clear();
	visited_refs.Clear();
	visited_defs.Clear();
	visited_decls.Clear();
}

void CodeVisitor::Visit(const String& filepath, const FileAnnotation& fa, Point begin,
                        Point end)
{
	Vector<Item> cur_level;
	for(int i = 0; i < fa.items.GetCount(); i++) {
		const AnnotationItem& ai = fa.items[i];
		if(begin.y <= ai.pos.y && ai.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.ann = &ai;
			it.file = filepath;
			it.pos = ai.pos;
		}
	}
	for(int i = 0; i < fa.locals.GetCount(); i++) {
		const AnnotationItem& ai = fa.locals[i];
		if(begin.y <= ai.pos.y && ai.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.ann = &ai;
			it.file = filepath;
			it.pos = ai.pos;
		}
	}
	for(int i = 0; i < fa.refs.GetCount(); i++) {
		const ReferenceItem& ref = fa.refs[i];
		if(begin.y <= ref.pos.y && ref.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.ref = &ref;
			it.file = filepath;
			it.pos = ref.pos;
		}
	}
	Sort(cur_level, Item());

	for(int i = 0; i < cur_level.GetCount(); i++) {
		Item& it = cur_level[i];
		LOG(i << ": " << it.ToString());
		if(it.ann) {
			const AnnotationItem& ai = *it.ann;
			if((ai.definition && visited_defs.Find(ai.id) >= 0) ||
			   (!ai.definition && visited_decls.Find(ai.id) >= 0))
				continue;
			export_items << it;
			Visit(filepath, ai);
		}
		if(it.ref) {
			const ReferenceItem& ref = *it.ref;
			String tgt_str = ref.MakeTargetString(filepath);
			if(visited_refs.Find(tgt_str) >= 0)
				continue;
			export_items << it;
			Visit(filepath, ref);
		}
		if(limit && export_items.GetCount() > limit)
			break;
	}
}

void CodeVisitor::Visit(const String& filepath, const AnnotationItem& ann)
{
	LOG(StoreAsJson(ann, true));
	if(ann.definition) {
		ASSERT(visited_defs.Find(ann.id) < 0);
		visited_defs.Add(ann.id);
		auto& codeidx = CodeIndex();
		int i = codeidx.Find(filepath);
		if(i < 0) {
			auto& it = export_items.Add();
			it.error_id = "error: file not found: " + filepath;
			it.file = filepath;
			it.pos = Point(0, 0);
			return;
		}
		const FileAnnotation& a = codeidx[i];
		Visit(filepath, a, ann.begin, ann.end);
	}
	else {
		ASSERT(visited_decls.Find(ann.id) < 0);
		visited_decls.Add(ann.id);
		VisitId(filepath, ann.id);
	}
}

void CodeVisitor::Visit(const String& filepath, const ReferenceItem& ref)
{
	LOG(StoreAsJson(ref, true));

	String tgt_str = ref.MakeTargetString(filepath);
	ASSERT(visited_refs.Find(tgt_str) < 0);
	visited_refs.Add(tgt_str);

	VisitId(filepath, ref.id);
}

void CodeVisitor::VisitId(const String& filepath, const String& cmp_id)
{
	// Find the reference
	auto& codeidx = CodeIndex();
	const FileAnnotation* match = 0;
	for(auto it : ~codeidx) {
		const FileAnnotation& a = it.value;
		for(const AnnotationItem& ann : a.items) {
			if(ann.definition && ann.id == cmp_id) {
				Visit(it.key, a, ann.begin, ann.end);
				return;
			}
		}
	}
	auto& it = export_items.Add();
	it.error_id = "error: id not found: " + cmp_id;
	it.pos = Point(0, 0);
	it.file = filepath;
}

const String& CodeVisitor::LoadPath(String path)
{
	int i = files.Find(path);
	if(i >= 0)
		return files[i];
	String& s = files.Add(path);
	s = LoadFile(path);
	return s;
}

END_UPP_NAMESPACE
