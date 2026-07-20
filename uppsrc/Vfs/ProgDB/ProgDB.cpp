#include "ProgDB.h"

NAMESPACE_UPP

ProgAstNode::ProgAstNode(const ProgAstNode& o) {
	type = o.type;
	content = o.content;
	children.Append(o.children);
	data = clone(o.data);
}

ProgAstNode& ProgAstNode::operator=(const ProgAstNode& o) {
	if (this != &o) {
		type = o.type;
		content = o.content;
		children.Clear();
		children.Append(o.children);
		data = clone(o.data);
	}
	return *this;
}

ProgNodeRecord::ProgNodeRecord(const ProgNodeRecord& o) {
	version = o.version;
	id = o.id;
	kind = o.kind;
	name = o.name;
	parent = o.parent;
	tags.Append(o.tags);
	for(int i = 0; i < o.relations.GetCount(); i++)
		relations.Add(o.relations[i]);
	data = clone(o.data);
	is_potential = o.is_potential;
	utility_level = o.utility_level;
	merge_policy = o.merge_policy;
	provenance = o.provenance;
	ast.Append(o.ast);
}

ProgNodeRecord& ProgNodeRecord::operator=(const ProgNodeRecord& o) {
	if (this != &o) {
		version = o.version;
		id = o.id;
		kind = o.kind;
		name = o.name;
		parent = o.parent;
		tags.Clear();
		tags.Append(o.tags);
		relations.Clear();
		for(int i = 0; i < o.relations.GetCount(); i++)
			relations.Add(o.relations[i]);
		data = clone(o.data);
		is_potential = o.is_potential;
		utility_level = o.utility_level;
		merge_policy = o.merge_policy;
		provenance = o.provenance;
		ast.Clear();
		ast.Append(o.ast);
	}
	return *this;
}

void ProgNodeRecord::Clear() {
	version = 1;
	id.value.Clear();
	kind = PROG_NODE_UNKNOWN;
	name.Clear();
	parent.value.Clear();
	tags.Clear();
	relations.Clear();
	data.Clear();

	is_potential = false;
	utility_level = PROG_UTILITY_EMIT;
	merge_policy = PROG_MERGE_APPEND;
	provenance.Clear();
	ast.Clear();
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

Value ProgAstNode::ToValue() const {
	ValueMap map;
	map.Set("type", type);
	map.Set("content", content);
	if(!children.IsEmpty()) {
		ValueArray arr;
		for(const auto& child : children)
			arr.Add(child.ToValue());
		map.Set("children", arr);
	}
	if(!data.IsEmpty())
		map.Set("data", data);
	return map;
}

bool ProgAstNode::Load(const Value& v) {
	if(!v.Is<ValueMap>()) return false;
	ValueMap map = v;
	Clear();
	type = map.Get("type", String());
	content = map.Get("content", String());
	Value children_val = map.Get("children", Value());
	if(children_val.Is<ValueArray>()) {
		ValueArray arr = children_val;
		for(int i = 0; i < arr.GetCount(); i++) {
			ProgAstNode& child = children.Add();
			child.Load(arr[i]);
		}
	}
	Value data_val = map.Get("data", Value());
	if(data_val.Is<ValueMap>())
		data = data_val;
	return true;
}

String ProgAstNode::ToLineFormat(int indent) const {
	String res;
	res << String(' ', indent) << type;
	if(!content.IsEmpty()) {
		res << " " << content;
	}
	res << "\n";
	for(const auto& child : children) {
		res << child.ToLineFormat(indent + 2);
	}
	return res;
}

bool ParseProgAst(const String& txt, Vector<ProgAstNode>& roots) {
	roots.Clear();
	Vector<String> lines = Split(txt, '\n');
	struct StackItem {
		int indent;
		ProgAstNode* node;
	};
	Vector<StackItem> stack;

	for(int i = 0; i < lines.GetCount(); i++) {
		String line;
		for(int c = 0; c < lines[i].GetCount(); c++) {
			if(lines[i][c] == '\t')
				line << "    ";
			else
				line.Cat(lines[i][c]);
		}
		
		int indent = 0;
		while(indent < line.GetCount() && line[indent] == ' ')
			indent++;
		
		String trimmed = TrimRight(line.Mid(indent));
		if(trimmed.IsEmpty() || trimmed.StartsWith("#") || trimmed.StartsWith("//"))
			continue;
		
		String type, content;
		int space_idx = trimmed.Find(' ');
		if(space_idx >= 0) {
			type = trimmed.Left(space_idx);
			content = trimmed.Mid(space_idx + 1);
		} else {
			type = trimmed;
		}

		ProgAstNode node;
		node.type = type;
		node.content = content;

		while(!stack.IsEmpty() && stack.Top().indent >= indent) {
			stack.Drop();
		}

		if(stack.IsEmpty()) {
			roots.Add(node);
			stack.Add({indent, &roots.Top()});
		} else {
			ProgAstNode& parent = *stack.Top().node;
			parent.children.Add(node);
			stack.Add({indent, &parent.children.Top()});
		}
	}
	return true;
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

	map.Set("is_potential", is_potential);
	map.Set("utility_level", (int)utility_level);
	map.Set("merge_policy", (int)merge_policy);
	
	ValueMap prov_map;
	prov_map.Set("source_file", provenance.source_file);
	prov_map.Set("line_number", provenance.line_number);
	prov_map.Set("generator_id", provenance.generator_id);
	prov_map.Set("potential_id", provenance.potential_id);
	map.Set("provenance", prov_map);

	if(!ast.IsEmpty()) {
		ValueArray ast_arr;
		for(const auto& node : ast)
			ast_arr.Add(node.ToValue());
		map.Set("ast", ast_arr);
	}

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

	is_potential = map.Get("is_potential", false);
	utility_level = (ProgUtilityLevel)int(map.Get("utility_level", (int)PROG_UTILITY_EMIT));
	merge_policy = (ProgMergePolicy)int(map.Get("merge_policy", (int)PROG_MERGE_APPEND));
	
	Value prov_val = map.Get("provenance", Value());
	if(prov_val.Is<ValueMap>()) {
		ValueMap prov_map = prov_val;
		provenance.source_file = prov_map.Get("source_file", String());
		provenance.line_number = prov_map.Get("line_number", 0);
		provenance.generator_id = prov_map.Get("generator_id", String());
		provenance.potential_id = prov_map.Get("potential_id", String());
	}
	
	Value ast_val = map.Get("ast", Value());
	if(ast_val.Is<ValueArray>()) {
		ValueArray ast_arr = ast_val;
		for(int i = 0; i < ast_arr.GetCount(); i++) {
			ProgAstNode& node = ast.Add();
			node.Load(ast_arr[i]);
		}
	}

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

String ProgNodeRecord::ToPLF() const {
	ASSERT(IsValid());
	String res;
	res << "id " << id.value << "\n";
	res << "kind " << ProgNodeKindToString(kind) << "\n";
	res << "name " << name << "\n";
	if(!parent.IsEmpty())
		res << "parent " << parent.value << "\n";
	res << "version " << version << "\n";
	res << "potential " << (is_potential ? "true" : "false") << "\n";
	res << "utility " << (int)utility_level << "\n";
	res << "merge " << (int)merge_policy << "\n";
	if(!provenance.source_file.IsEmpty() || provenance.line_number > 0 || !provenance.generator_id.IsEmpty() || !provenance.potential_id.IsEmpty()) {
		res << "provenance " << provenance.source_file << ":" << provenance.line_number 
		    << " generator=" << provenance.generator_id << " potential=" << provenance.potential_id << "\n";
	}
	for(const String& tag : tags)
		res << "tag " << tag << "\n";
	for(const ProgRelation& rel : relations)
		res << "relation " << rel.type << " " << rel.target.value << "\n";
	
	for(int i = 0; i < data.GetCount(); i++) {
		res << "data " << data.GetKey(i) << " " << AsTOON(data.GetValue(i), 0, ' ', false) << "\n";
	}

	if(!ast.IsEmpty()) {
		res << "ast:\n";
		for(const auto& node : ast) {
			res << node.ToLineFormat(2);
		}
	}
	return res;
}

bool ProgNodeRecord::LoadPLF(const String& plf) {
	Clear();
	Vector<String> lines = Split(plf, '\n');
	bool parsing_ast = false;
	String ast_txt;

	for(int i = 0; i < lines.GetCount(); i++) {
		String line = lines[i];
		if(parsing_ast) {
			int indent = 0;
			while(indent < line.GetCount() && line[indent] == ' ')
				indent++;
			
			String trimmed = TrimBoth(line);
			if(trimmed.IsEmpty()) {
				ast_txt << "\n";
				continue;
			}
			
			if(indent == 0) {
				parsing_ast = false;
				ParseProgAst(ast_txt, ast);
			} else {
				ast_txt << line << "\n";
				continue;
			}
		}

		String trimmed = TrimBoth(line);
		if(trimmed.IsEmpty() || trimmed.StartsWith("#") || trimmed.StartsWith("//"))
			continue;

		int space_idx = trimmed.Find(' ');
		String key, val;
		if(space_idx >= 0) {
			key = trimmed.Left(space_idx);
			val = trimmed.Mid(space_idx + 1);
		} else {
			key = trimmed;
		}

		if(key == "id") {
			id.value = val;
		} else if(key == "kind") {
			kind = StringToProgNodeKind(val);
		} else if(key == "name") {
			name = val;
		} else if(key == "parent") {
			parent.value = val;
		} else if(key == "version") {
			version = StrInt(val);
		} else if(key == "potential") {
			is_potential = (val == "true");
		} else if(key == "utility") {
			utility_level = (ProgUtilityLevel)StrInt(val);
		} else if(key == "merge") {
			merge_policy = (ProgMergePolicy)StrInt(val);
		} else if(key == "provenance") {
			int colon = val.Find(':');
			if(colon >= 0) {
				provenance.source_file = val.Left(colon);
				int gen_idx = val.Find(" generator=");
				if(gen_idx >= 0) {
					provenance.line_number = StrInt(val.Mid(colon + 1, gen_idx - (colon + 1)));
					int pot_idx = val.Find(" potential=");
					if(pot_idx >= 0) {
						provenance.generator_id = val.Mid(gen_idx + 11, pot_idx - (gen_idx + 11));
						provenance.potential_id = val.Mid(pot_idx + 11);
					} else {
						provenance.generator_id = val.Mid(gen_idx + 11);
					}
				} else {
					provenance.line_number = StrInt(val.Mid(colon + 1));
				}
			}
		} else if(key == "tag") {
			tags.Add(val);
		} else if(key == "relation") {
			int r_space = val.Find(' ');
			if(r_space >= 0) {
				ProgRelation& rel = relations.Add();
				rel.type = val.Left(r_space);
				rel.target.value = val.Mid(r_space + 1);
			}
		} else if(key == "data") {
			int d_space = val.Find(' ');
			if(d_space >= 0) {
				String d_key = val.Left(d_space);
				String d_val = val.Mid(d_space + 1);
				Value parsed = ParseTOON(~d_val, true, 2, false);
				if(!parsed.IsError()) {
					data.Set(d_key, parsed);
				}
			}
		} else if(key == "ast:") {
			parsing_ast = true;
			ast_txt.Clear();
		}
	}

	if(parsing_ast && !ast_txt.IsEmpty()) {
		ParseProgAst(ast_txt, ast);
	}

	return IsValid();
}

static String GetNodeDir(ProgNodeKind kind) {
	switch(kind) {
	case PROG_NODE_UNIT:           return "units";
	case PROG_NODE_NAMESPACE:
	case PROG_NODE_CLASS:
	case PROG_NODE_STRUCT:
	case PROG_NODE_ENUM:
	case PROG_NODE_VARIABLE:       return "symbols";
	case PROG_NODE_FUNCTION:       return "functions";
	case PROG_NODE_COMMENT:        return "comments";
	case PROG_NODE_TAG:            return "tags";
	case PROG_NODE_TARGET_BINDING: return "target-api-map";
	default:                       return "unknown";
	}
}

static String GetNodeFilename(const ProgNodeId& id) {
	String fn = id.value;
	for(int i = 0; i < fn.GetCount(); i++) {
		char c = fn[i];
		if(!IsAlNum(c) && c != '.' && c != '_' && c != '-') {
			fn.Set(i, '_');
		}
	}
	return fn << ".plf";
}

static String GetNodeFilePath(const String& root, const ProgNodeRecord& rec) {
	String dir = AppendFileName(root, GetNodeDir(rec.kind));
	return AppendFileName(dir, GetNodeFilename(rec.id));
}

static void LoadNodesFromDir(const String& dir, ArrayMap<ProgNodeId, ProgNodeRecord>& nodes) {
	FindFile ff(AppendFileName(dir, "*"));
	while(ff) {
		if(ff.IsFile()) {
			String path = ff.GetPath();
			String ext = GetFileExt(path);
			if(ext == ".plf" || ext == ".toon") {
				String content = LoadFile(path);
				ProgNodeRecord rec;
				bool loaded = false;
				if(ext == ".plf") {
					loaded = rec.LoadPLF(content);
				} else {
					loaded = rec.LoadTOON(content);
				}
				if(loaded) {
					nodes.GetAdd(rec.id) = rec;
				}
			}
		}
		ff.Next();
	}
}

bool ProgDatabase::Open(const String& path) {
	root_dir = path;
	nodes.Clear();
	if(root_dir.IsEmpty()) return false;

	RealizeDirectory(AppendFileName(root_dir, "units"));
	RealizeDirectory(AppendFileName(root_dir, "symbols"));
	RealizeDirectory(AppendFileName(root_dir, "functions"));
	RealizeDirectory(AppendFileName(root_dir, "comments"));
	RealizeDirectory(AppendFileName(root_dir, "tags"));
	RealizeDirectory(AppendFileName(root_dir, "target-api-map"));

	LoadNodesFromDir(AppendFileName(root_dir, "units"), nodes);
	LoadNodesFromDir(AppendFileName(root_dir, "symbols"), nodes);
	LoadNodesFromDir(AppendFileName(root_dir, "functions"), nodes);
	LoadNodesFromDir(AppendFileName(root_dir, "comments"), nodes);
	LoadNodesFromDir(AppendFileName(root_dir, "tags"), nodes);
	LoadNodesFromDir(AppendFileName(root_dir, "target-api-map"), nodes);

	RebuildIndexes();
	return true;
}

bool ProgDatabase::Save() {
	if(root_dir.IsEmpty()) return false;
	for(int i = 0; i < nodes.GetCount(); i++) {
		const ProgNodeRecord& rec = nodes[i];
		if(!rec.IsValid()) continue;
		String path = GetNodeFilePath(root_dir, rec);
		String content = rec.ToPLF();
		SaveFile(path, content);
	}
	return true;
}

bool ProgDatabase::AddNode(const ProgNodeRecord& rec) {
	if(!rec.IsValid()) return false;
	nodes.GetAdd(rec.id) = rec;
	RebuildIndexes();
	return true;
}

bool ProgDatabase::RemoveNode(const ProgNodeId& id) {
	int idx = nodes.Find(id);
	if(idx < 0) return false;
	const ProgNodeRecord& rec = nodes[idx];
	String path = GetNodeFilePath(root_dir, rec);
	FileDelete(path);
	nodes.Remove(idx);
	RebuildIndexes();
	return true;
}

bool ProgDatabase::RenameNode(const ProgNodeId& id, const String& new_name) {
	int idx = nodes.Find(id);
	if(idx < 0) return false;
	ProgNodeRecord rec = nodes[idx];
	String old_path = GetNodeFilePath(root_dir, rec);
	FileDelete(old_path);

	rec.name = new_name;
	nodes[idx] = rec;
	
	String new_path = GetNodeFilePath(root_dir, rec);
	SaveFile(new_path, rec.ToPLF());
	
	RebuildIndexes();
	return true;
}

bool ProgDatabase::RenameNodeId(const ProgNodeId& old_id, const ProgNodeId& new_id) {
	int idx = nodes.Find(old_id);
	if(idx < 0) return false;
	
	ProgNodeRecord rec = nodes[idx];
	String old_path = GetNodeFilePath(root_dir, rec);
	FileDelete(old_path);
	
	rec.id = new_id;
	nodes.Remove(idx);
	nodes.Add(new_id, rec);
	
	for(int i = 0; i < nodes.GetCount(); i++) {
		ProgNodeRecord& other = nodes[i];
		if(other.parent == old_id) {
			other.parent = new_id;
		}
		for(auto& rel : other.relations) {
			if(rel.target == old_id) {
				rel.target = new_id;
			}
		}
	}
	
	String new_path = GetNodeFilePath(root_dir, rec);
	SaveFile(new_path, rec.ToPLF());
	
	RebuildIndexes();
	return true;
}

void ProgDatabase::RebuildIndexes() {
	units.Clear();
	symbols.Clear();
	functions.Clear();
	comments.Clear();
	tags.Clear();
	target_bindings.Clear();

	for(int i = 0; i < nodes.GetCount(); i++) {
		const ProgNodeRecord& rec = nodes[i];
		switch(rec.kind) {
		case PROG_NODE_UNIT:           units.Add(rec.id); break;
		case PROG_NODE_NAMESPACE:
		case PROG_NODE_CLASS:
		case PROG_NODE_STRUCT:
		case PROG_NODE_ENUM:
		case PROG_NODE_VARIABLE:       symbols.Add(rec.id); break;
		case PROG_NODE_FUNCTION:       functions.Add(rec.id); break;
		case PROG_NODE_COMMENT:        comments.Add(rec.id); break;
		case PROG_NODE_TAG:            tags.Add(rec.id); break;
		case PROG_NODE_TARGET_BINDING: target_bindings.Add(rec.id); break;
		default:                       break;
		}
	}
}

const ProgNodeRecord* ProgDatabase::FindNode(const ProgNodeId& id) const {
	int idx = nodes.Find(id);
	return idx >= 0 ? &nodes[idx] : nullptr;
}

ProgNodeRecord* ProgDatabase::GetNodeWritable(const ProgNodeId& id) {
	int idx = nodes.Find(id);
	return idx >= 0 ? &nodes[idx] : nullptr;
}

Vector<ProgNodeId> ProgDatabase::QueryByKind(ProgNodeKind kind) const {
	Vector<ProgNodeId> res;
	for(int i = 0; i < nodes.GetCount(); i++) {
		if(nodes[i].kind == kind) {
			res.Add(nodes[i].id);
		}
	}
	return res;
}

Vector<ProgNodeId> ProgDatabase::QueryChildren(const ProgNodeId& parent_id) const {
	Vector<ProgNodeId> res;
	for(int i = 0; i < nodes.GetCount(); i++) {
		if(nodes[i].parent == parent_id) {
			res.Add(nodes[i].id);
		}
	}
	return res;
}

Vector<ProgNodeId> ProgDatabase::QueryByTag(const String& tag) const {
	Vector<ProgNodeId> res;
	for(int i = 0; i < nodes.GetCount(); i++) {
		bool has_tag = false;
		for(int t = 0; t < nodes[i].tags.GetCount(); t++) {
			if(nodes[i].tags[t] == tag) {
				has_tag = true;
				break;
			}
		}
		if(has_tag) {
			res.Add(nodes[i].id);
		}
	}
	return res;
}

Vector<ProgNodeId> ProgDatabase::QueryRelations(const String& rel_type, const ProgNodeId& target_id) const {
	Vector<ProgNodeId> res;
	for(int i = 0; i < nodes.GetCount(); i++) {
		const ProgNodeRecord& rec = nodes[i];
		for(const auto& rel : rec.relations) {
			if(rel.type == rel_type && rel.target == target_id) {
				res.Add(rec.id);
				break;
			}
		}
	}
	return res;
}

END_UPP_NAMESPACE
