#ifndef _Vfs_ProgDB_ProgDB_h_
#define _Vfs_ProgDB_ProgDB_h_

#include <Core/Core.h>
#include <Vfs/Core/Core.h>

NAMESPACE_UPP

enum ProgNodeKind {
	PROG_NODE_UNKNOWN,
	PROG_NODE_UNIT,
	PROG_NODE_NAMESPACE,
	PROG_NODE_CLASS,
	PROG_NODE_STRUCT,
	PROG_NODE_ENUM,
	PROG_NODE_FUNCTION,
	PROG_NODE_VARIABLE,
	PROG_NODE_STATEMENT,
	PROG_NODE_COMMENT,
	PROG_NODE_TAG,
	PROG_NODE_TARGET_BINDING
};

enum ProgUtilityLevel {
	PROG_UTILITY_POOL,
	PROG_UTILITY_EMIT,
	PROG_UTILITY_UTILITY,
	PROG_UTILITY_FROZEN
};

enum ProgMergePolicy {
	PROG_MERGE_PREPEND,
	PROG_MERGE_APPEND,
	PROG_MERGE_REPLACE,
	PROG_MERGE_EXCLUSIVE
};

struct ProgNodeId : Moveable<ProgNodeId> {
	String value;

	ProgNodeId() {}
	ProgNodeId(const String& val) : value(val) {}
	ProgNodeId(const char* val) : value(val) {}

	bool IsEmpty() const          { return value.IsEmpty(); }
	String ToString() const      { return value; }

	bool operator==(const ProgNodeId& o) const { return value == o.value; }
	bool operator!=(const ProgNodeId& o) const { return value != o.value; }
	unsigned GetHashValue() const { return UPP::GetHashValue(value); }
};

struct ProgNodeRef : Moveable<ProgNodeRef> {
	ProgNodeId id;
	String     path;
};

struct ProgRelation : Moveable<ProgRelation> {
	String     type;
	ProgNodeId target;
};

struct ProgProvenance : Moveable<ProgProvenance> {
	String source_file;
	int    line_number = 0;
	String generator_id;
	String potential_id;

	void Clear() {
		source_file.Clear();
		line_number = 0;
		generator_id.Clear();
		potential_id.Clear();
	}
};

struct ProgAstNode : Moveable<ProgAstNode> {
	String               type;
	String               content;
	Vector<ProgAstNode>  children;
	ValueMap             data;

	void Clear() {
		type.Clear();
		content.Clear();
		children.Clear();
		data.Clear();
	}

	ProgAstNode() {}
	ProgAstNode(const ProgAstNode& o);
	ProgAstNode& operator=(const ProgAstNode& o);

	Value ToValue() const;
	bool Load(const Value& v);
	String ToLineFormat(int indent = 0) const;
};

struct ProgNodeRecord : Moveable<ProgNodeRecord> {
	int                  version = 1;
	ProgNodeId           id;
	ProgNodeKind         kind = PROG_NODE_UNKNOWN;
	String               name;
	ProgNodeId           parent;
	Vector<String>       tags;
	Vector<ProgRelation> relations;
	ValueMap             data;

	// Potential/Actual/Meta metadata
	bool                 is_potential = false;
	ProgUtilityLevel     utility_level = PROG_UTILITY_EMIT;
	ProgMergePolicy      merge_policy = PROG_MERGE_APPEND;
	ProgProvenance       provenance;
	Vector<ProgAstNode>  ast;

	ProgNodeRecord() {}
	ProgNodeRecord(const ProgNodeRecord& o);
	ProgNodeRecord& operator=(const ProgNodeRecord& o);

	void Clear();
	bool IsValid() const;
	Value ToValue() const;
	bool Load(const Value& value);
	String ToTOON() const;
	bool LoadTOON(const String& toon);

	// Custom Line Format (PLF)
	String ToPLF() const;
	bool LoadPLF(const String& plf);
};

String ProgNodeKindToString(ProgNodeKind kind);
ProgNodeKind StringToProgNodeKind(const String& kind);

bool ParseProgAst(const String& txt, Vector<ProgAstNode>& nodes);

class ProgDatabase {
	String root_dir;
	ArrayMap<ProgNodeId, ProgNodeRecord> nodes;
	
	// Indexes for fast queries
	Index<ProgNodeId> units;
	Index<ProgNodeId> symbols;
	Index<ProgNodeId> functions;
	Index<ProgNodeId> comments;
	Index<ProgNodeId> tags;
	Index<ProgNodeId> target_bindings;

	void RebuildIndexes();

public:
	ProgDatabase() {}
	
	bool Open(const String& path);
	bool Save();
	void Close() { nodes.Clear(); root_dir.Clear(); RebuildIndexes(); }

	bool AddNode(const ProgNodeRecord& rec);
	bool RemoveNode(const ProgNodeId& id);
	bool RenameNode(const ProgNodeId& id, const String& new_name);
	bool RenameNodeId(const ProgNodeId& old_id, const ProgNodeId& new_id);

	const ProgNodeRecord* FindNode(const ProgNodeId& id) const;
	ProgNodeRecord* GetNodeWritable(const ProgNodeId& id);
	
	Vector<ProgNodeId> QueryByKind(ProgNodeKind kind) const;
	Vector<ProgNodeId> QueryChildren(const ProgNodeId& parent_id) const;
	Vector<ProgNodeId> QueryByTag(const String& tag) const;
	Vector<ProgNodeId> QueryRelations(const String& rel_type, const ProgNodeId& target_id) const;
	
	String GetRootDirectory() const { return root_dir; }
	int GetNodeCount() const { return nodes.GetCount(); }
};

END_UPP_NAMESPACE

#endif
