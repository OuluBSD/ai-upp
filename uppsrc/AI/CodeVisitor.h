#ifndef _AI_CodeVisitor_h_
#define _AI_CodeVisitor_h_

NAMESPACE_UPP

struct CodeVisitorProfile
{
	typedef enum : int {
		VISIT_VARIABLE_USAGE,
		VISIT_VARIABLE_TYPE_REQ_DEF,
		VISIT_VARIABLE_TYPE_DECL,
		
		RULE_COUNT,
	} Rule;
	bool cfg[RULE_COUNT];
	
	CodeVisitorProfile();
	CodeVisitorProfile(const CodeVisitorProfile& p) {*this = p;}
	void operator=(const CodeVisitorProfile& p);
	bool Is(Rule r) const;
	CodeVisitorProfile& Set(Rule r, bool b);
	CodeVisitorProfile& SetAllTrue();
};

struct CodeVisitor
{
	struct Item : Moveable<Item> {
		bool have_ann = false;
		bool have_ref = false;
		bool have_link = false;
		AnnotationItem ann;
		ReferenceItem ref;
		AnnotationItem link;
		String error;
		Point pos;
		String file;
		String link_file;
		
		Item() {}
		Item(const Item& it) {*this = it;}
		void operator=(const Item& it);
		bool operator()(const Item& a, const Item& b) const;
		String ToString() const;
	};
	int limit = 0;
	CodeVisitorProfile prof;
	Vector<Item> items;
	Vector<Item> export_items;
	VectorMap<String,String> files;
	Index<String> visited_refs;
	Index<String> visited_defs;
	Index<String> visited_decls;
	
	CodeVisitor() {}
	void SetLimit(int i) {limit = i;}
	void SetNoLimit() {limit = 0;}
	void SetProfile(CodeVisitorProfile& p) {prof = p;}
	void Begin();
	void Visit(const String& filepath, const FileAnnotation& fa, Point begin, Point end);
	void VisitAnn(const String& filepath, Item& it);
	void VisitRef(const String& filepath, Item& it);
	void VisitId(const String& filepath, const String& id, Item& it);
	const String& LoadPath(String path);
};

END_UPP_NAMESPACE

#endif
