#include "Script.h"

namespace Upp {
namespace Node {

// ---------------------------------------------------------------------------
// Internal: node type definition collected from 'node' top-level declarations
// ---------------------------------------------------------------------------

struct PinDef : Moveable<PinDef> {
	String name;
	String type;   // e.g. "MODEL", "CLIP", "LATENT"
	bool   is_out = false;
};

struct ParamDef : Moveable<ParamDef> {
	String name;
	String type;
	String default_val;
	String widget;
};

struct NodeTypeDef : Moveable<NodeTypeDef> {
	String              type_id;   // e.g. "comfyui.gguf.unet_loader"
	Array<PinDef>       pins;
	Array<ParamDef>     params;
};

// ---------------------------------------------------------------------------
// Internal: group definition collected from // GROUP: ... comment annotations
// ---------------------------------------------------------------------------

struct GroupDef : Moveable<GroupDef> {
	String        vfs_path;
	String        label;
	GroupStyle    style;
};

// Forward declarations
static GroupStyle ParseGroupStyle(const String& style_str);

// ---------------------------------------------------------------------------
// Internal: node position annotations collected from // POS: comments
// ---------------------------------------------------------------------------

struct PosAnnotation : Moveable<PosAnnotation> {
	String node_id;
	double x = 0, y = 0, w = 0, h = 0;
};

static Vector<PosAnnotation> ParsePosAnnotations(const String& eon_text)
{
	Vector<PosAnnotation> result;
	const char* p   = eon_text.Begin();
	const char* end = eon_text.End();
	while (p < end) {
		if (*p == '/' && p + 1 < end && *(p+1) == '/') {
			p += 2;
			String line;
			while (p < end && *p != '\n' && *p != '\r')
				line << *p++;
			int idx = line.Find("POS:");
			if (idx >= 0 && TrimBoth(line.Left(idx)).IsEmpty()) {
				String rest = TrimBoth(line.Mid(idx + 4));
				// rest = "node_id x y [w h]"
				Vector<String> parts = Split(rest, ' ');
				if (parts.GetCount() >= 3) {
					PosAnnotation a;
					a.node_id = parts[0];
					a.x = ScanDouble(parts[1]);
					a.y = ScanDouble(parts[2]);
					if (parts.GetCount() >= 5) {
						a.w = ScanDouble(parts[3]);
						a.h = ScanDouble(parts[4]);
					}
					result.Add(pick(a));
				}
			}
		}
		else {
			while (p < end && *p != '\n') p++;
		}
		if (p < end) p++; // skip newline
	}
	return result;
}

// Parse group annotations from EON file comments
// Format: // GROUP: group_id {saturation:N, hue:name, contrast:N}
static Vector<GroupDef> ParseGroupAnnotations(const String& eon_text)
{
	Vector<GroupDef> groups;
	
	const char* p = eon_text.Begin();
	const char* end = eon_text.End();
	
	while (p < end) {
		// Look for "// GROUP:"
		if (*p == '/' && *(p+1) == '/') {
			const char* line_start = p;
			p += 2;
			
			String line;
			while (p < end && *p != '\n' && *p != '\r') {
				line << *p++;
			}
			
			int idx = line.Find("GROUP:");
			if (idx >= 0) {
				// Make sure GROUP is right after the comment prefix "//"
				String before_group = TrimBoth(line.Left(idx));
				if(before_group != "" && before_group != "//") 
					continue; // Something other than "//" before GROUP:, skip
				
				String rest = TrimBoth(line.Mid(idx + 6));
				GroupDef gd;
				
				// Extract group id (first word before space or '{')
				int space = rest.Find(' ');
				int brace = rest.Find('{');
				int delim = -1;
				if(space >= 0 && brace >= 0)
					delim = space < brace ? space : brace;
				else if(space >= 0)
					delim = space;
				else if(brace >= 0)
					delim = brace;
				
				if (delim > 0) {
					gd.vfs_path = "/" + TrimBoth(rest.Left(delim));
					String after_id = TrimBoth(rest.Mid(delim));
					gd.label = gd.vfs_path.Mid(1); // default label = path segment
					// Optional quoted label: // GROUP: enc "Some Label" {style}
					if(after_id.GetCount() > 0 && after_id[0] == '"') {
						int close = after_id.Find('"', 1);
						if(close > 1) {
							gd.label = after_id.Mid(1, close - 1);
							after_id = TrimBoth(after_id.Mid(close + 1));
						}
					}
					// Extract style part inside {}
					int startBrace = after_id.Find('{');
					int endBrace   = after_id.Find('}');
					String style_part;
					if(startBrace >= 0 && endBrace > startBrace)
						style_part = after_id.Mid(startBrace + 1, endBrace - startBrace - 1);
					else if(startBrace >= 0)
						style_part = after_id.Mid(startBrace + 1);
					gd.style = ParseGroupStyle(style_part);
				} else if (brace < 0 && space < 0) {
					gd.vfs_path = "/" + rest;
					gd.label = gd.vfs_path.Mid(1);
				} else {
					// Just '{' without group id - skip
					continue;
				}
				
				if (!gd.vfs_path.IsEmpty())
					groups.Add(gd);
			}
		}
		
		// Advance to next line
		while (p < end && *p != '\n' && *p != '\r') p++;
		if (p < end) p++;
	}
	
	return groups;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Parse a "|"-separated field string: "a|b|c|d" -> parts[0..3]
static Vector<String> SplitPipe(const String& s, int n)
{
	Vector<String> parts;
	int pos = 0;
	for (int i = 0; i < n - 1; i++) {
		int sep = s.Find('|', pos);
		if (sep < 0) { parts.Add(s.Mid(pos)); pos = s.GetCount(); break; }
		parts.Add(s.Mid(pos, sep - pos));
		pos = sep + 1;
	}
	parts.Add(s.Mid(pos));
	while (parts.GetCount() < n) parts.Add(String());
	return parts;
}

static void CollectNodeTypeDefs(const AstNode& root,
                                 VectorMap<String, NodeTypeDef>& types)
{
	for (const AstNode& child : const_cast<AstNode&>(root).val.Sub<AstNode>()) {
		if (child.GetName() == "node_type_decl") {
			NodeTypeDef& def = types.GetAdd(child.id.ToString());
			def.type_id = child.id.ToString();

			for (const AstNode& item : const_cast<AstNode&>(child).val.Sub<AstNode>()) {
				String name = item.GetName();
				if (name == "pin_in" || name == "pin_out") {
					Vector<String> parts = SplitPipe(item.str, 2);
					PinDef& p = def.pins.Add();
					p.name   = parts[0];
					p.type   = parts[1];
					p.is_out = (name == "pin_out");
				}
				else if (name == "node_param") {
					Vector<String> parts = SplitPipe(item.str, 4);
					ParamDef& p = def.params.Add();
					p.name        = parts[0];
					p.type        = parts[1];
					p.default_val = parts[2];
					p.widget      = parts[3];
				}
			}
		}
		else {
			CollectNodeTypeDefs(child, types);
		}
	}
}

static Color NamedColor(const String& name)
{
	static const struct { const char* n; Color c; } map[] = {
		{ "purple", Color(160, 32, 240) }, { "yellow", Yellow()           },
		{ "orange", Color(255, 128, 0)  }, { "pink",   Color(255,182,193) },
		{ "blue",   Blue()              }, { "green",  Green()            },
		{ "red",    Red()               }, { "cyan",   Cyan()             },
		{ "white",  White()             }, { "black",  Black()            },
	};
	for (auto& e : map)
		if (name == e.n) return e.c;
	return Null;
}

static bool ParseInt(const String& s, int& result)
{
	result = 0;
	bool neg = false;
	int i = 0;
	if(i < s.GetCount() && s[i] == '-') { neg = true; i++; }
	if(i >= s.GetCount()) return false;
	for(; i < s.GetCount(); i++) {
		char c = s[i];
		if(c < '0' || c > '9') return false;
		result = result * 10 + (c - '0');
	}
	if(neg) result = -result;
	return true;
}

static GroupStyle ParseGroupStyle(const String& style_str)
{
	GroupStyle s;
	if(style_str.IsEmpty()) return s; 
	String str = style_str;
	str.Replace("{", "");
	str.Replace("}", "");
	Vector<String> props = Split(str, ',');
	for(String& prop : props) {
		prop = TrimBoth(prop);
		int colon = prop.Find(':');
		if(colon < 0) continue;
		String key = TrimBoth(prop.Left(colon));
		String val = TrimBoth(prop.Mid(colon + 1));
		if(key == "saturation") {
			val.Replace("%", "");
			int v;
			if(ParseInt(TrimBoth(val), v) && v >= 0 && v <= 100) s.saturation = v;
		}
		else if(key == "hue") {
			if(val == "red") s.hue = 0;
			else if(val == "orange") s.hue = 30;
			else if(val == "yellow") s.hue = 60;
			else if(val == "green") s.hue = 120;
			else if(val == "cyan") s.hue = 180;
			else if(val == "blue") s.hue = 240;
			else if(val == "purple") s.hue = 270;
			else {
				int v;
				if(ParseInt(TrimBoth(val), v) && v >= 0 && v <= 360) s.hue = v;
			}
		}
		else if(key == "contrast") {
			val.Replace("%", "");
			int v;
			if(ParseInt(TrimBoth(val), v) && v >= 0 && v <= 100) s.contrast = v;
		}
	}
	return s;
}

static Color PinTypeColor(const String& type_name)
{
	if (type_name == "MODEL")        return Color(175, 100, 255);
	if (type_name == "CLIP")         return Color(255, 220,  50);
	if (type_name == "CONDITIONING") return Color(255, 150,  50);
	if (type_name == "LATENT")       return Color(255, 180, 200);
	if (type_name == "VAE")          return Color(220,  80,  80);
	if (type_name == "IMAGE")        return Color( 80, 210, 100);
	if (type_name == "INT")          return Color(150, 200, 255);
	if (type_name == "FLOAT")        return Color(200, 230, 180);
	if (type_name == "STRING")       return Color(200, 200, 200);
	return Color(160, 160, 160);
}

static void ApplyPins(NodeDoc& nd, const NodeTypeDef& def)
{
	for (const PinDef& p : def.pins) {
		PinDoc& pin = nd.pins.Add();
		pin.id        = p.name;
		pin.label     = p.name;
		pin.kind      = p.is_out ? PinKind::Output : PinKind::Input;
		pin.type_name = p.type;
		pin.color     = PinTypeColor(p.type);
	}
}

static void SplitNodePort(const Graph& g, const String& full,
                          String& node_id, String& port_id)
{
	int dot = full.ReverseFind('.');
	if (dot < 0) { node_id = full; port_id = ""; return; }
	String maybe_node = full.Left(dot);
	if (g.FindNode(maybe_node)) {
		node_id = maybe_node;
		port_id = full.Mid(dot + 1);
	} else {
		node_id = full;
		port_id = "";
	}
}

static String ExtractParamValue(const AstNode& expr_stmt)
{
	if (!expr_stmt.rval) return String();
	const AstNode* assign_op = expr_stmt.rval;
	if (!assign_op->arg[1]) return String();
	const AstNode* val_node = assign_op->arg[1];
	if (!val_node->str.IsEmpty()) return val_node->str;
	if (IsPartially(val_node->src, Cursor_Literal_INT32) ||
	    IsPartially(val_node->src, Cursor_Literal_BOOL))
		return IntStr((int)val_node->i64);
	if (IsPartially(val_node->src, Cursor_Literal_DOUBLE))
		return DblStr(val_node->dbl);
	return String();
}

static String ExtractParamKey(const AstNode& expr_stmt)
{
	if (!expr_stmt.rval) return String();
	const AstNode* assign_op = expr_stmt.rval;
	if (!assign_op->arg[0]) return String();
	const AstNode* rval = assign_op->arg[0];
	if (rval->rval) return rval->rval->GetName();
	return rval->GetName(); 
}

static String CollectPath(const AstNode& node) {
	String s = node.GetName();
	if (s.IsEmpty()) s = node.str;
	if (node.src == Cursor_NamePart || node.src == Cursor_Unresolved) {
		for (const AstNode& child : const_cast<AstNode&>(node).val.Sub<AstNode>()) {
			String cs = CollectPath(child);
			if (!cs.IsEmpty()) {
				if (!s.IsEmpty()) s << ".";
				s << cs;
			}
		}
	}
	return s;
}

// Forward declarations
static void LoadEonRecursive(const AstNode& node, String prefix, Graph& g,
                             const VectorMap<String, NodeTypeDef>& types,
                             Vector<ValidationMessage>& out,
                             bool& found_net,
                             NodeDoc* parent_node = nullptr);

static void LoadNetCompound(const AstNode& compound, Graph& g,
                            const VectorMap<String, NodeTypeDef>& types,
                            Vector<ValidationMessage>& out)
{
	for (const AstNode& stmt : const_cast<AstNode&>(compound).val.Sub<AstNode>()) {
		if (stmt.src == Cursor_VarDecl) continue;

		if (stmt.src == Cursor_AtomStmt || stmt.src == Cursor_NamePart) {
			bool found_net_dummy = false;
			LoadEonRecursive(stmt, "", g, types, out, found_net_dummy);
			continue;
		}

		if (stmt.src == Cursor_ExprStmt) {
			const String& cs = stmt.str;
			if (cs.Find("->") >= 0) {
				int arrow = cs.Find("->");
				String src_full = TrimBoth(cs.Left(arrow));
				String dst_full = TrimBoth(cs.Mid(arrow + 2));

				String src_node, src_pin, dst_node, dst_pin;
				SplitNodePort(g, src_full, src_node, src_pin);
				SplitNodePort(g, dst_full, dst_node, dst_pin);
				if (src_node.IsEmpty() || dst_node.IsEmpty()) continue;

				auto EnsurePin = [&](const String& nid, const String& pid, PinKind kind) {
					NodeDoc* nd = g.FindNode(nid);
					if (nd && !pid.IsEmpty()) {
						bool found = false;
						for (const auto& p : nd->pins) if (p.id == pid && p.kind == kind) { found = true; break; }
						if (!found) {
							PinDoc& p = nd->pins.Add();
							p.id = pid;
							p.label = pid;
							p.kind = kind;
							p.color = PinTypeColor(""); 
						}
					}
				};
				EnsurePin(src_node, src_pin, PinKind::Output);
				EnsurePin(dst_node, dst_pin, PinKind::Input);

				String eid_raw = "e_" + src_node + "_" + src_pin + "_" + dst_node + "_" + dst_pin;
				String eid = eid_raw;
				eid.Replace(".", "_");
				EdgeDoc& ed = g.AddEdge(eid, src_node, src_pin, dst_node, dst_pin);
				ed.directed = true;

				if (!stmt.obj.IsVoid()) {
					ValueMap meta = stmt.obj;
					Color clr = NamedColor(meta["color"].ToString());
					if (!IsNull(clr)) ed.stroke_clr = clr;
					Value wv = meta["weight"];
					if (!wv.IsVoid()) ed.weight = (double)wv;
				}
				continue;
			}

			if (stmt.rval && stmt.rval->src == Cursor_Unresolved) {
				String atom_id = CollectPath(*stmt.rval);
				if(atom_id.IsEmpty()) atom_id = stmt.rval->id.ToString();
				if(atom_id[0] == '/') atom_id = atom_id.Mid(1);
				if(IsValidEntityId(atom_id)) {
					NodeDoc& nd = g.AddNode(atom_id);
					nd.label = atom_id;
					nd.node_type_id = atom_id;
					{
						int dot = atom_id.Find('.');
						nd.category = (dot >= 0) ? atom_id.Left(dot) : atom_id;
					}
					int current_type_idx = types.Find(atom_id);
					if (current_type_idx >= 0)
						ApplyPins(nd, types[current_type_idx]);
					continue;
				}
			}
		}
	}

	NodeDoc* current_atom = nullptr;
	for (const AstNode& stmt : const_cast<AstNode&>(compound).val.Sub<AstNode>()) {
		if (stmt.src == Cursor_AtomStmt || stmt.src == Cursor_NamePart) {
			String atom_id = CollectPath(stmt);
			if(atom_id.IsEmpty()) atom_id = stmt.id.ToString();
			if(atom_id[0] == '/') atom_id = atom_id.Mid(1);
			current_atom = g.FindNode(atom_id);
			if (current_atom && stmt.src == Cursor_AtomStmt) {
				for (const AstNode& achild : const_cast<AstNode&>(stmt).val.Sub<AstNode>()) {
					if (achild.src == Cursor_CompoundStmt) {
						for (const AstNode& p : const_cast<AstNode&>(achild).val.Sub<AstNode>()) {
							if (p.src == Cursor_ExprStmt) {
								String key = ExtractParamKey(p);
								if (!key.IsEmpty()) {
									String val = ExtractParamValue(p);
									WidgetSlotDoc& slot = current_atom->slots.Add();
									slot.id = key;
									slot.type = "param";
									slot.properties.Add("value", val);
								}
							}
						}
					}
				}
			}
			continue;
		}
		if (stmt.src == Cursor_ExprStmt) {
			if (stmt.str.Find("->") >= 0) { current_atom = nullptr; continue; }
			if (!current_atom) continue;
			String key = ExtractParamKey(stmt);
			if (key.IsEmpty()) continue;
			String val = ExtractParamValue(stmt);
			WidgetSlotDoc& slot = current_atom->slots.Add();
			slot.id = key;
			slot.type = "param";
			slot.properties.Add("value", val);
		}
	}
}

static void LoadEonRecursive(const AstNode& node, String prefix, Graph& g,
                             const VectorMap<String, NodeTypeDef>& types,
                             Vector<ValidationMessage>& out,
                             bool& found_net,
                             NodeDoc* parent_node)
{
	if (node.src == Cursor_NetStmt) {
		for (const AstNode& child : const_cast<AstNode&>(node).val.Sub<AstNode>()) {
			if (child.src == Cursor_CompoundStmt)
				LoadNetCompound(child, g, types, out);
		}
		found_net = true;
		return;
	}

	String name = node.GetName();
	if (name.IsEmpty()) name = node.str;
	if (name.IsEmpty() && node.src != Cursor_CompoundStmt)
		name = node.id.ToString();

	String full = prefix;
	if (!name.IsEmpty()) {
		if (!full.IsEmpty()) full << ".";
		full << name;
	}
	if (full.StartsWith(".")) full = full.Mid(1);
	if (!full.IsEmpty() && full[0] == '/') full = full.Mid(1);

	bool is_atom   = (node.src == Cursor_AtomStmt);
	bool is_entity = (node.src == Cursor_EntityStmt);
	bool is_system = (node.src == Cursor_SystemStmt);
	bool is_comp   = (node.src == Cursor_ComponentStmt);
	bool is_leaf   = node.val.Sub<AstNode>().IsEmpty();

	NodeDoc* current_node = nullptr;

	if (is_comp && parent_node) {
		WidgetSlotDoc& slot = parent_node->slots.Add();
		slot.id = name;
		slot.type = "comp";
		for (const AstNode& achild : const_cast<AstNode&>(node).val.Sub<AstNode>()) {
			if (achild.src == Cursor_CompoundStmt) {
				for (const AstNode& p : const_cast<AstNode&>(achild).val.Sub<AstNode>()) {
					if (p.src == Cursor_ExprStmt) {
						String key = ExtractParamKey(p);
						if (!key.IsEmpty()) {
							String val = ExtractParamValue(p);
							slot.properties.Add(key, val);
						}
					}
				}
			}
		}
		return; 
	}

	if (is_atom || is_entity || is_system || (node.src == Cursor_NamePart && is_leaf)) {
		if (IsValidEntityId(full)) {
			String type_id = full;
			if (is_atom || is_entity || is_system) {
				for (const AstNode& achild : const_cast<AstNode&>(node).val.Sub<AstNode>()) {
					if (achild.src != Cursor_CompoundStmt) continue;
					for (const AstNode& gc : const_cast<AstNode&>(achild).val.Sub<AstNode>()) {
						if (gc.src != Cursor_ExprStmt) continue;
						const AstNode* unres = gc.rval;
						if (unres) {
							String t = CollectPath(*unres);
							if (!t.IsEmpty()) { type_id = t; break; }
						}
					}
					if (type_id != full) break;
				}
			}

			NodeDoc& nd = g.AddNode(full);
			nd.label = name;
			nd.node_type_id = type_id;
			{
				int dot = type_id.Find('.');
				nd.category = (dot >= 0) ? type_id.Left(dot) : type_id;
			}
			if (is_system) nd.category = "system";
			int current_type_idx = types.Find(type_id);
			if (current_type_idx >= 0)
				ApplyPins(nd, types[current_type_idx]);
			current_node = &nd;
		}
	}

	if (node.src == Cursor_WorldStmt || node.src == Cursor_PoolStmt) {
		String grp_id = full;
		grp_id.Replace(".", "_");
		GroupDoc& grp = g.AddGroup(grp_id);
		grp.label = name;
		grp.vfs_path = "/" + full;
	}

	for (const AstNode& child : const_cast<AstNode&>(node).val.Sub<AstNode>()) {
		LoadEonRecursive(child, full, g, types, out, found_net, current_node ? current_node : parent_node);
	}
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool LoadEon(Graph& g, const String& eon_text,
             Vector<ValidationMessage>& out)
{
	Vector<GroupDef>    group_defs = ParseGroupAnnotations(eon_text);
	Vector<PosAnnotation> pos_defs = ParsePosAnnotations(eon_text);

	VfsValue root_val;
	Compiler compiler(root_val);
	
	Vector<ValidationMessage> parse_errors;
	compiler.WhenMessage = Callback1<ProcMsg>([&](const ProcMsg& m) {
		if (m.severity == PROCMSG_ERROR)
			parse_errors.Add(ValidationMessage(ValidationMessage::ERROR, m.msg + " (" + IntStr(m.line) + ":" + IntStr(m.col) + ")"));
	}, 1);

	AstNode* sem_root = compiler.CompileToSemantic(eon_text, "<eon>", false);
	if (!sem_root) {
		for (const auto& m : parse_errors) out.Add(m);
		out.Add(ValidationMessage(ValidationMessage::ERROR, "Eon parse failed"));
		return false;
	}

	VectorMap<String, NodeTypeDef> types;
	CollectNodeTypeDefs(*sem_root, types);

	bool found_net = false;
	for (const AstNode& top : sem_root->val.Sub<AstNode>()) {
		LoadEonRecursive(top, "", g, types, out, found_net);
	}

	for(int i = 0; i < group_defs.GetCount(); i++) {
		const GroupDef& gd = group_defs[i];
		String grp_id = gd.vfs_path.Mid(1);
		if(grp_id.IsEmpty()) grp_id = "group_" + IntStr(i);
		grp_id.Replace(".", "_");
		
		GroupDoc* grp = g.FindGroup(grp_id);
		if(!grp) grp = &g.AddGroup(grp_id);
		
		grp->label = gd.label;
		grp->vfs_path = gd.vfs_path;
		grp->style = gd.style;

		String prefix = gd.vfs_path.Mid(1);
		for(auto& node : g.GetDoc().nodes) {
			if(node.id.StartsWith(prefix + ".") || node.id == prefix) {
				bool already = false;
				for(const auto& nid : grp->nodes) if(nid == node.id) { already = true; break; }
				if(!already) grp->nodes.Add(node.id);
			}
		}
	}
	
	for(auto& grp : g.GetDoc().groups) {
		String prefix = grp.vfs_path.Mid(1);
		if(prefix.IsEmpty()) continue;
		for(auto& node : g.GetDoc().nodes) {
			if(node.id.StartsWith(prefix + ".") || node.id == prefix) {
				bool already = false;
				for(const auto& nid : grp.nodes) if(nid == node.id) { already = true; break; }
				if(!already) const_cast<GroupDoc&>(grp).nodes.Add(node.id);
			}
		}
	}

	for (const PosAnnotation& pa : pos_defs) {
		NodeDoc* nd = g.FindNode(pa.node_id);
		if (!nd) continue;
		nd->pos.x = pa.x;
		nd->pos.y = pa.y;
		if (pa.w > 0) nd->sz.cx = pa.w;
		if (pa.h > 0) nd->sz.cy = pa.h;
	}

	if (!found_net)
		out.Add(ValidationMessage(ValidationMessage::WARNING,
		        "No net block found in eon file"));

	g.RebuildIndexPublic();
	Vector<ValidationMessage> val_msgs = g.Validate();
	bool has_error = false;
	for (const auto& m : val_msgs) {
		out.Add(m);
		if (m.severity == ValidationMessage::ERROR)
			has_error = true;
	}
	return !has_error;
}

bool LoadEonFile(Graph& g, const String& path,
                 Vector<ValidationMessage>& out)
{
	if (!FileExists(path)) {
		out.Add(ValidationMessage(ValidationMessage::ERROR,
		        "File not found: " + path));
		return false;
	}
	String content = LoadFile(path);
	if (content.IsEmpty()) {
		out.Add(ValidationMessage(ValidationMessage::ERROR,
		        "Empty file: " + path));
		return false;
	}
	return LoadEon(g, content, out);
}

String SaveEon(const Graph& g)
{
	const GraphDoc& doc = g.GetDoc();
	String s;

	Index<String> emitted_types;
	for (const NodeDoc& nd : doc.nodes) {
		if (nd.pins.IsEmpty() && nd.slots.IsEmpty()) continue;
		String type_id = nd.node_type_id.IsEmpty() ? nd.label : nd.node_type_id;
		if (type_id.IsEmpty() || emitted_types.Find(type_id) >= 0) continue;
		emitted_types.Add(type_id);

		s << "node " << type_id << ":\n";
		for (const PinDoc& pin : nd.pins) {
			s << "\t" << (pin.kind == PinKind::Output ? "out" : "in")
			  << " " << pin.id;
			if (!pin.type_name.IsEmpty())
				s << " : " << pin.type_name;
			s << "\n";
		}
		for (WidgetSlotDoc& slot : const_cast<NodeDoc&>(nd).slots) {
			s << "\t" << slot.id;
			if (!slot.type.IsEmpty() && slot.type != "param")
				s << " : " << slot.type;
			Value v = slot.properties.Get("value", Value());
			if (!v.IsVoid())
				s << " = " << v.ToString();
			s << "\n";
		}
		s << "\n";
	}

	s << "net graph:\n";

	for (const NodeDoc& nd : doc.nodes) {
		String type_id = nd.node_type_id.IsEmpty() ? nd.label : nd.node_type_id;
		s << "\t" << nd.id << ": " << type_id << "\n";
		for (WidgetSlotDoc& slot : const_cast<NodeDoc&>(nd).slots) {
			Value v = slot.properties.Get("value", Value());
			s << "\t\t" << slot.id << " = ";
			if (IsString(v))
				s << "\"" << v.ToString() << "\"";
			else if (!v.IsVoid())
				s << v.ToString();
			else
				s << "\"\"";
			s << "\n";
		}
	}
	s << "\n";

	for (const EdgeDoc& ed : doc.edges) {
		s << "\t" << ed.source_node;
		if (!ed.source_pin.IsEmpty()) s << "." << ed.source_pin;
		s << " -> " << ed.target_node;
		if (!ed.target_pin.IsEmpty()) s << "." << ed.target_pin;
		s << "\n";
	}

	bool has_positions = false;
	for (const NodeDoc& nd : doc.nodes)
		if (nd.pos.x != 0.0 || nd.pos.y != 0.0) { has_positions = true; break; }
	if (has_positions) {
		s << "\n";
		for (const NodeDoc& nd : doc.nodes)
			s << "// POS: " << nd.id << " " << nd.pos.x << " " << nd.pos.y
			  << " " << nd.sz.cx << " " << nd.sz.cy << "\n";
	}

	return s;
}

} // namespace Node
} // namespace Upp
