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
	struct Item {
		Ptr<MetaNode> node;
		Ptr<MetaNode> link_node;
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
	Array<Item> export_items;
	VectorMap<String,String> files;
	Index<MetaNode*> visited;
	Vector<MetaNode*> macro_exps, macro_defs;
	
	CodeVisitor() {}
	void SetLimit(int i) {limit = i;}
	void SetNoLimit() {limit = 0;}
	void SetProfile(CodeVisitorProfile& p) {prof = p;}
	void Begin();
	void Visit(const String& filepath, MetaNode& n);
	void VisitSub(const String& filepath, MetaNode& n);
	void VisitRef(const String& filepath, MetaNode& it);
	void VisitId(const String& filepath, MetaNode& n, Item& it);
	const String& LoadPath(String path);
	int FindItem(MetaNode* n) const;
};

END_UPP_NAMESPACE

#endif
