#include "AI.h"

NAMESPACE_UPP

CodeVisitorProfile::CodeVisitorProfile() {
	memset(&cfg, 0, sizeof(cfg));
}

void CodeVisitorProfile::operator=(const CodeVisitorProfile& p) {
	memcpy(&cfg, &p.cfg, sizeof(cfg));
}

bool CodeVisitorProfile::Is(Rule r) const {
	ASSERT((int)r >= 0 && r < RULE_COUNT);
	return cfg[r];
}

CodeVisitorProfile& CodeVisitorProfile::Set(Rule r, bool b) {
	ASSERT((int)r >= 0 && r < RULE_COUNT);
	cfg[r] = b;
	return *this;
}

CodeVisitorProfile& CodeVisitorProfile::SetAllTrue() {
	memset(&cfg, 0xFF, sizeof(cfg));
	return *this;
}





void CodeVisitor::Item::operator=(const Item& it) {
	have_ann	= it.have_ann;
	have_ref	= it.have_ref;
	have_link	= it.have_link;
	ann			= it.ann;
	ref			= it.ref;
	link		= it.link;
	error		= it.error;
	pos			= it.pos;
	file		= it.file;
	link_file	= it.link_file;
}

bool CodeVisitor::Item::operator()(const Item& a, const Item& b) const
{
	if (a.file != b.file) return a.file < b.file;
	if (a.pos.y != b.pos.y) return a.pos.y < b.pos.y;
	return a.pos.x < b.pos.x;
}

String CodeVisitor::Item::ToString() const
{
	String s;
	if (have_ann)  {s << ann.ToString();}
	if (have_ref)  {if (!s.IsEmpty()) s << ", "; s << ref.ToString();}
	if (have_link) {if (!s.IsEmpty()) s << ", "; s << link.ToString();}
	return s;
}

void CodeVisitor::Begin()
{
	items.Clear();
	export_items.Clear();
	visited_refs.Clear();
	visited_defs.Clear();
	visited_decls.Clear();
}

void CodeVisitor::Visit(const String& filepath, const FileAnnotation& fa, Point begin, Point end)
{
	Vector<Item> cur_level;
	for(int i = 0; i < fa.items.GetCount(); i++) {
		const AnnotationItem& ai = fa.items[i];
		if(begin.y <= ai.pos.y && ai.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.have_ann = true;
			it.ann = ai;
			it.file = filepath;
			it.pos = ai.pos;
		}
	}
	for(int i = 0; i < fa.locals.GetCount(); i++) {
		const AnnotationItem& ai = fa.locals[i];
		if(begin.y <= ai.pos.y && ai.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.have_ann = true;
			it.ann = ai;
			it.file = filepath;
			it.pos = ai.pos;
		}
	}
	for(int i = 0; i < fa.refs.GetCount(); i++) {
		const ReferenceItem& ref = fa.refs[i];
		if(begin.y <= ref.pos.y && ref.pos.y <= end.y) {
			Item& it = cur_level.Add();
			it.have_ref = true;
			it.ref = ref;
			it.file = filepath;
			it.pos = ref.pos;
		}
	}
	Sort(cur_level, Item());

	for(int i = 0; i < cur_level.GetCount(); i++) {
		Item& it = cur_level[i];
		LOG(i << ": " << it.ToString());
		if(it.have_ann) {
			const AnnotationItem& ai = it.ann;
			if((ai.definition && visited_defs.Find(ai.id) >= 0) ||
			   (!ai.definition && visited_decls.Find(ai.id) >= 0))
				continue;
			export_items << it;
			VisitAnn(filepath, it);
		}
		if(it.have_ref) {
			const ReferenceItem& ref = it.ref;
			String tgt_str = ref.MakeTargetString(filepath);
			if(visited_refs.Find(tgt_str) >= 0)
				continue;
			export_items << it;
			VisitRef(filepath, it);
		}
		if(limit && export_items.GetCount() > limit)
			break;
	}
}

void CodeVisitor::VisitAnn(const String& filepath, Item& it)
{
	ASSERT(it.have_ann);
	const AnnotationItem& ann = it.ann;
	//LOG(StoreAsJson(ann, true));
	if(ann.definition) {
		ASSERT(visited_defs.Find(ann.id) < 0);
		visited_defs.Add(ann.id);
		auto& codeidx = CodeIndex();
		int i = codeidx.Find(filepath);
		if(i < 0) {
			auto& it = export_items.Add();
			it.error = "file not found: " + filepath;
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
		VisitId(filepath, ann.id, it);
	}
}

void CodeVisitor::VisitRef(const String& filepath, Item& it)
{
	ASSERT(it.have_ref);
	const ReferenceItem& ref = it.ref;
	//LOG(StoreAsJson(ref, true));

	String tgt_str = ref.MakeTargetString(filepath);
	ASSERT(visited_refs.Find(tgt_str) < 0);
	visited_refs.Add(tgt_str);
	
	VisitId(filepath, ref.id, it);
}

void CodeVisitor::VisitId(const String& filepath, const String& cmp_id, Item& link_it)
{
	// Find the reference
	auto& codeidx = CodeIndex();
	const FileAnnotation* match = 0;
	for(auto it : ~codeidx) {
		const FileAnnotation& a = it.value;
		Vector<const Vector<AnnotationItem>*> lists;
		lists << &a.items;
		lists << &a.locals;
		for (const Vector<AnnotationItem>* list : lists) {
			for(const AnnotationItem& ann : *list) {
				if(ann.definition && ann.id == cmp_id) {
					link_it.link = ann;
					link_it.link_file = it.key;
					link_it.have_link = true;
					Visit(it.key, a, ann.begin, ann.end);
					return;
				}
			}
		}
	}
	for(auto it : ~codeidx) {
		const FileAnnotation& a = it.value;
		Vector<const Vector<AnnotationItem>*> lists;
		lists << &a.items;
		lists << &a.locals;
		for (const Vector<AnnotationItem>* list : lists) {
			for(const AnnotationItem& ann : *list) {
				if(!ann.definition && ann.id == cmp_id) {
					link_it.link = ann;
					link_it.link_file = it.key;
					link_it.have_link = true;
					Visit(it.key, a, ann.begin, ann.end);
					return;
				}
			}
		}
	}
	auto& it = export_items.Add();
	it.error = "id not found: " + cmp_id;
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
