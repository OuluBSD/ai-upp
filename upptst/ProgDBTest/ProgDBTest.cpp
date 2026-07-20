#include <Core/Core.h>
#include <Vfs/ProgDB/ProgDB.h>
#include <iostream>
#include <cstdio>

using namespace Upp;

static void TestNodeRecordPLFParity() {
	ProgNodeRecord rec;
	rec.id.value = "class.Upp.String";
	rec.kind = PROG_NODE_CLASS;
	rec.name = "String";
	rec.parent.value = "namespace.Upp";
	rec.tags.Add("core");
	rec.tags.Add("text");

	ProgRelation& r = rec.relations.Add();
	r.type = "inherits";
	r.target.value = "class.Upp.Moveable";

	rec.data.Set("custom_key", "custom_value");
	rec.data.Set("number_key", 42);

	rec.is_potential = true;
	rec.utility_level = PROG_UTILITY_POOL;
	rec.merge_policy = PROG_MERGE_EXCLUSIVE;
	rec.provenance.source_file = "String.fog";
	rec.provenance.line_number = 123;
	rec.provenance.generator_id = "gen01";
	rec.provenance.potential_id = "pot01";

	// Add AST statements
	ProgAstNode& ast_if = rec.ast.Add();
	ast_if.type = "if";
	ast_if.content = "x > 10";
	
	ProgAstNode& ast_call = ast_if.children.Add();
	ast_call.type = "call";
	ast_call.content = "print(\"hello\")";

	String plf = rec.ToPLF();
	LOG("Generated PLF:\n" << plf);

	ProgNodeRecord rec2;
	ASSERT(rec2.LoadPLF(plf));

	ASSERT(rec2.id == rec.id);
	ASSERT(rec2.kind == rec.kind);
	ASSERT(rec2.name == rec.name);
	ASSERT(rec2.parent == rec.parent);
	ASSERT(rec2.tags.GetCount() == 2);
	ASSERT(rec2.tags[0] == "core");
	ASSERT(rec2.tags[1] == "text");
	ASSERT(rec2.relations.GetCount() == 1);
	ASSERT(rec2.relations[0].type == "inherits");
	ASSERT(rec2.relations[0].target.value == "class.Upp.Moveable");
	ASSERT(rec2.data["custom_key"] == "custom_value");
	ASSERT(rec2.data["number_key"] == 42);
	ASSERT(rec2.is_potential == rec.is_potential);
	ASSERT(rec2.utility_level == rec.utility_level);
	ASSERT(rec2.merge_policy == rec.merge_policy);
	ASSERT(rec2.provenance.source_file == rec.provenance.source_file);
	ASSERT(rec2.provenance.line_number == rec.provenance.line_number);
	ASSERT(rec2.provenance.generator_id == rec.provenance.generator_id);
	ASSERT(rec2.provenance.potential_id == rec.provenance.potential_id);

	ASSERT(rec2.ast.GetCount() == 1);
	ASSERT(rec2.ast[0].type == "if");
	ASSERT(rec2.ast[0].content == "x > 10");
	ASSERT(rec2.ast[0].children.GetCount() == 1);
	ASSERT(rec2.ast[0].children[0].type == "call");
	ASSERT(rec2.ast[0].children[0].content == "print(\"hello\")");
}

static void TestIndentDedentParser() {
	String code = 
		"class String\n"
		"  method GetCount\n"
		"    return len\n"
		"  method IsEmpty\n"
		"    if len == 0\n"
		"      return true\n"
		"    else\n"
		"      return false\n";

	Vector<ProgAstNode> roots;
	ASSERT(ParseProgAst(code, roots));

	ASSERT(roots.GetCount() == 1);
	ASSERT(roots[0].type == "class");
	ASSERT(roots[0].content == "String");
	ASSERT(roots[0].children.GetCount() == 2);

	const ProgAstNode& get_count = roots[0].children[0];
	ASSERT(get_count.type == "method");
	ASSERT(get_count.content == "GetCount");
	ASSERT(get_count.children.GetCount() == 1);
	ASSERT(get_count.children[0].type == "return");
	ASSERT(get_count.children[0].content == "len");

	const ProgAstNode& is_empty = roots[0].children[1];
	ASSERT(is_empty.type == "method");
	ASSERT(is_empty.content == "IsEmpty");
	ASSERT(is_empty.children.GetCount() == 2);

	const ProgAstNode& if_node = is_empty.children[0];
	ASSERT(if_node.type == "if");
	ASSERT(if_node.content == "len == 0");
	ASSERT(if_node.children.GetCount() == 1);
	ASSERT(if_node.children[0].type == "return");
	ASSERT(if_node.children[0].content == "true");

	const ProgAstNode& else_node = is_empty.children[1];
	ASSERT(else_node.type == "else");
	ASSERT(else_node.children.GetCount() == 1);
	ASSERT(else_node.children[0].type == "return");
	ASSERT(else_node.children[0].content == "false");

	String reconstructed = roots[0].ToLineFormat(0);
	LOG("Reconstructed AST:\n" << reconstructed);

	Vector<ProgAstNode> roots2;
	ASSERT(ParseProgAst(reconstructed, roots2));
	ASSERT(roots2.GetCount() == 1);
	ASSERT(roots2[0].type == "class");
	ASSERT(roots2[0].content == "String");
	ASSERT(roots2[0].children.GetCount() == 2);
}

static void TestDatabaseOperations() {
	String temp_dir = AppendFileName(GetTempDirectory(), "progdb_test_dir");
	RealizeDirectory(temp_dir);
	LOG("Database test dir: " << temp_dir);

	{
		ProgDatabase db;
		ASSERT(db.Open(temp_dir));

		ProgNodeRecord unit;
		unit.id.value = "unit.main";
		unit.kind = PROG_NODE_UNIT;
		unit.name = "main";
		ASSERT(db.AddNode(unit));

		ProgNodeRecord cls;
		cls.id.value = "class.Upp.String";
		cls.kind = PROG_NODE_CLASS;
		cls.name = "String";
		cls.parent.value = "unit.main";
		cls.tags.Add("text");
		ASSERT(db.AddNode(cls));

		ProgNodeRecord func;
		func.id.value = "function.Upp.String.GetCount";
		func.kind = PROG_NODE_FUNCTION;
		func.name = "GetCount";
		func.parent.value = "class.Upp.String";
		
		ProgRelation& rel = func.relations.Add();
		rel.type = "belongs_to";
		rel.target.value = "class.Upp.String";
		
		ASSERT(db.AddNode(func));

		ASSERT(db.Save());
	}

	// Read it back
	{
		ProgDatabase db;
		ASSERT(db.Open(temp_dir));
		ASSERT(db.GetNodeCount() == 3);

		const ProgNodeRecord* unit = db.FindNode(ProgNodeId{"unit.main"});
		ASSERT(unit);
		ASSERT(unit->kind == PROG_NODE_UNIT);

		const ProgNodeRecord* cls = db.FindNode(ProgNodeId{"class.Upp.String"});
		ASSERT(cls);
		ASSERT(cls->parent.value == "unit.main");
		ASSERT(FindIndex(cls->tags, "text") >= 0);

		Vector<ProgNodeId> children = db.QueryChildren(ProgNodeId{"class.Upp.String"});
		ASSERT(children.GetCount() == 1);
		ASSERT(children[0].value == "function.Upp.String.GetCount");

		Vector<ProgNodeId> text_nodes = db.QueryByTag("text");
		ASSERT(text_nodes.GetCount() == 1);
		ASSERT(text_nodes[0].value == "class.Upp.String");

		Vector<ProgNodeId> rels = db.QueryRelations("belongs_to", ProgNodeId{"class.Upp.String"});
		ASSERT(rels.GetCount() == 1);
		ASSERT(rels[0].value == "function.Upp.String.GetCount");

		// Test Rename
		ASSERT(db.RenameNode(ProgNodeId{"class.Upp.String"}, "NewString"));
		const ProgNodeRecord* renamed_cls = db.FindNode(ProgNodeId{"class.Upp.String"});
		ASSERT(renamed_cls);
		ASSERT(renamed_cls->name == "NewString");

		// Test Rename Node ID with Referential Integrity
		ASSERT(db.RenameNodeId(ProgNodeId{"class.Upp.String"}, ProgNodeId{"class.Upp.NewString"}));
		ASSERT(db.FindNode(ProgNodeId{"class.Upp.String"}) == nullptr);
		const ProgNodeRecord* moved_cls = db.FindNode(ProgNodeId{"class.Upp.NewString"});
		ASSERT(moved_cls);
		ASSERT(moved_cls->name == "NewString");

		// Check parent references and relations were updated in other nodes!
		const ProgNodeRecord* moved_func = db.FindNode(ProgNodeId{"function.Upp.String.GetCount"});
		ASSERT(moved_func);
		ASSERT(moved_func->parent.value == "class.Upp.NewString");
		ASSERT(moved_func->relations.GetCount() == 1);
		ASSERT(moved_func->relations[0].target.value == "class.Upp.NewString");

		// Clean up by removing a node
		ASSERT(db.RemoveNode(ProgNodeId{"unit.main"}));
		ASSERT(db.FindNode(ProgNodeId{"unit.main"}) == nullptr);
	}

	// Clean up database directory
	DeleteFolderDeep(temp_dir);
}

CONSOLE_APP_MAIN {
	TestNodeRecordPLFParity();
	TestIndentDedentParser();
	TestDatabaseOperations();
}
