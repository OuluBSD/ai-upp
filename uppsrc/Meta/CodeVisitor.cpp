#include "Meta.h"

NAMESPACE_UPP

bool IsStruct(int kind)
{
	return findarg(kind, CXCursor_StructDecl, CXCursor_UnionDecl, CXCursor_ClassDecl,
	                     CXCursor_ClassTemplate) >= 0;
}

bool IsFunction(int kind)
{
	return findarg(kind, CXCursor_FunctionTemplate, CXCursor_FunctionDecl, CXCursor_Constructor,
	                     CXCursor_Destructor, CXCursor_ConversionFunction, CXCursor_CXXMethod) >= 0;
}



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
	node		= it.node;
	link_node	= it.link_node;
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
	if(node) {
		if (node->id.GetCount() && node->type.GetCount()) s << node->id << ": " << node->type;
		else if (node->id.GetCount()) s << node->id;
		else if (node->type.GetCount()) s << node->type;
	}
	return s;
}

void CodeVisitor::Begin()
{
	export_items.Clear();
	visited.Clear();
	// TODO optimize: cache this
	macro_exps = MetaEnv().root.FindAllShallow(CXCursor_MacroExpansion);
	macro_defs = MetaEnv().root.FindAllShallow(CXCursor_MacroDefinition);
}

int CodeVisitor::FindItem(MetaNode* n) const {
	int i = 0;
	for (const auto& it : export_items) {
		if (&*it.node == n)
			return i;
		i++;
	}
	return -1;
}

void CodeVisitor::Visit(const String& filepath, MetaNode& n)
{
	if (visited.Find(&n) >= 0)
		return;
	visited.Add(&n);
	
	/*if (n.kind == CXCursor_UnexposedExpr) {
		LOG(n.GetTreeString());
	}*/
	
	bool add_item = false;
	if ((!n.id.IsEmpty() || !n.type.IsEmpty()) /*&& FindItem(&n) < 0*/ &&
		n.kind != CXCursor_CXXBaseSpecifier)
		add_item = true;
	else if (n.kind == CXCursor_ReturnStmt ||
			 n.kind == CXCursor_MacroDefinition ||
			 n.kind == CXCursor_MacroExpansion)
		add_item = true;
	
	if (add_item) {
		Item& it = export_items.Add();
		it.pos = n.begin;
		it.file = filepath;
		it.node = &n;
		
		// Visit macro definition in "brute-forced" way
		if (n.kind == CXCursor_MacroExpansion) {
			String id = n.id;
			for (MetaNode* md : macro_defs) {
				if (md->id == id) {
					it.link_node = md;
					Visit(filepath, *md);
				}
			}
		}
	}
	
	VisitSub(filepath, n);
	
	// Macro expansions are not in the node-structure already, and they must be "brute-force" visited
	if (IsStruct(n.kind) || IsFunction(n.kind)) {
		int pkg = n.pkg;
		int file = n.file;
		for (MetaNode* me : macro_exps) {
			if (me->pkg == pkg && me->file == file) {
				if (RangeContains(me->begin, n.begin, n.end)) {
					Visit(filepath, *me);
				}
			}
		}
	}
}

void CodeVisitor::VisitSub(const String& filepath, MetaNode& n) {
	for(int i = 0; i < n.sub.GetCount(); i++) {
		MetaNode& s = n.sub[i];
		
		if (!s.is_ref) {
			Visit(filepath, s);
		}
		else {
			VisitRef(filepath, s);
		}
		if(limit && export_items.GetCount() > limit)
			break;
	}
}

void CodeVisitor::VisitRef(const String& filepath, MetaNode& n)
{
	ASSERT(n.is_ref);
	/*if (!n.id.IsEmpty() || !n.type.IsEmpty()) {
	}*/
	/*if (FindItem(&n) < 0)*/ {
		Item& it = export_items.Add();
		it.pos = n.begin;
		it.file = filepath;
		it.node = &n;
		VisitId(filepath, n, it);
	}
}

void CodeVisitor::VisitId(const String& filepath, MetaNode& n, Item& link_it)
{
	VisitSub(filepath, n);
	
	ASSERT(n.is_ref);
	auto& env = MetaEnv();
	MetaNode* decl = env.FindDeclaration(n);
	if (decl) {
		link_it.link_node = decl;
		String filepath = env.GetFilepath(decl->pkg, decl->file);
		Visit(filepath, *decl);
	}
	else /*if (FindItem(&n) < 0)*/ {
		auto& it = export_items.Add();
		it.error = "id not found: " + n.id;
		it.pos = Point(0, 0);
		it.file = filepath;
		it.node = &n;
	}
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
