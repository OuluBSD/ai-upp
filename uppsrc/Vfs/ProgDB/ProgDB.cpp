#include "ProgDB.h"

NAMESPACE_UPP

void ProgNodeRecord::Clear() {
	version = 1;
	id.value.Clear();
	kind = PROG_NODE_UNKNOWN;
	name.Clear();
	parent.value.Clear();
	tags.Clear();
	relations.Clear();
	data.Clear();
}

bool ProgNodeRecord::IsValid() const {
	return !id.IsEmpty() && kind != PROG_NODE_UNKNOWN;
}

String ProgNodeKindToString(ProgNodeKind kind) {
	switch(kind) {
	case PROG_NODE_UNIT:           return "unit";
	case PROG_NODE_NAMESPACE:      return "namespace";
	case PROG_NODE_CLASS:          return "class";
	case PROG_NODE_STRUCT:         return "struct";
	case PROG_NODE_ENUM:           return "enum";
	case PROG_NODE_FUNCTION:       return "function";
	case PROG_NODE_VARIABLE:       return "variable";
	case PROG_NODE_STATEMENT:      return "statement";
	case PROG_NODE_COMMENT:        return "comment";
	case PROG_NODE_TAG:            return "tag";
	case PROG_NODE_TARGET_BINDING: return "target_binding";
	default:                       return "unknown";
	}
}

ProgNodeKind StringToProgNodeKind(const String& kind) {
	if(kind == "unit")           return PROG_NODE_UNIT;
	if(kind == "namespace")      return PROG_NODE_NAMESPACE;
	if(kind == "class")          return PROG_NODE_CLASS;
	if(kind == "struct")         return PROG_NODE_STRUCT;
	if(kind == "enum")           return PROG_NODE_ENUM;
	if(kind == "function")       return PROG_NODE_FUNCTION;
	if(kind == "variable")       return PROG_NODE_VARIABLE;
	if(kind == "statement")      return PROG_NODE_STATEMENT;
	if(kind == "comment")        return PROG_NODE_COMMENT;
	if(kind == "tag")            return PROG_NODE_TAG;
	if(kind == "target_binding") return PROG_NODE_TARGET_BINDING;
	return PROG_NODE_UNKNOWN;
}

Value ProgNodeRecord::ToValue() const {
	ValueMap map;
	map.Set("version", version);
	map.Set("id", id.value);
	map.Set("kind", ProgNodeKindToString(kind));
	map.Set("name", name);
	if(!parent.IsEmpty())
		map.Set("parent", parent.value);
	if(!tags.IsEmpty()) {
		ValueArray arr;
		for(const String& tag : tags)
			arr.Add(tag);
		map.Set("tags", arr);
	}
	if(!relations.IsEmpty()) {
		ValueArray arr;
		for(const ProgRelation& rel : relations) {
			ValueMap row;
			row.Set("type", rel.type);
			row.Set("target", rel.target.value);
			arr.Add(row);
		}
		map.Set("relations", arr);
	}
	if(!data.IsEmpty())
		map.Set("data", data);
	return map;
}

bool ProgNodeRecord::Load(const Value& value) {
	if(!value.Is<ValueMap>())
		return false;
	ValueMap map = value;
	Clear();
	version = map.Get("version", 1);
	id.value = map.Get("id", String());
	kind = StringToProgNodeKind(map.Get("kind", String()));
	name = map.Get("name", String());
	parent.value = map.Get("parent", String());
	Value tag_value = map.Get("tags", Value());
	if(tag_value.Is<ValueArray>()) {
		ValueArray arr = tag_value;
		for(int i = 0; i < arr.GetCount(); i++)
			tags.Add(AsString(arr[i]));
	}
	Value rel_value = map.Get("relations", Value());
	if(rel_value.Is<ValueArray>()) {
		ValueArray arr = rel_value;
		for(int i = 0; i < arr.GetCount(); i++) {
			if(!arr[i].Is<ValueMap>())
				return false;
			ValueMap row = arr[i];
			ProgRelation& rel = relations.Add();
			rel.type = row.Get("type", String());
			rel.target.value = row.Get("target", String());
		}
	}
	Value data_value = map.Get("data", Value());
	if(data_value.Is<ValueMap>())
		data = data_value;
	return IsValid();
}

String ProgNodeRecord::ToTOON() const {
	ASSERT(IsValid());
	return AsTOON(ToValue(), 2, ',', false);
}

bool ProgNodeRecord::LoadTOON(const String& toon) {
	Value value = ParseTOON(~toon, true, 2, false);
	if(value.IsError())
		return false;
	return Load(value);
}

END_UPP_NAMESPACE
