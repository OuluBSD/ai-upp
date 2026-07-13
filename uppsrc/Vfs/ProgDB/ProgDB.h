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

struct ProgNodeId : Moveable<ProgNodeId> {
	String value;

	bool IsEmpty() const          { return value.IsEmpty(); }
	String ToString() const      { return value; }
};

struct ProgNodeRef : Moveable<ProgNodeRef> {
	ProgNodeId id;
	String     path;
};

struct ProgRelation : Moveable<ProgRelation> {
	String     type;
	ProgNodeId target;
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

	void Clear();
	bool IsValid() const;
	Value ToValue() const;
	bool Load(const Value& value);
	String ToTOON() const;
	bool LoadTOON(const String& toon);
};

String ProgNodeKindToString(ProgNodeKind kind);
ProgNodeKind StringToProgNodeKind(const String& kind);

END_UPP_NAMESPACE

#endif
