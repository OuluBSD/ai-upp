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

// Map VFS path to group (for nodes with hierarchical paths like /enc/node1)
static String GetGroupPathForNode(const String& node_id, const VectorMap<String, GroupDef>& groups)
{
	// Try longest prefix match
	String best_match;
	for(int i = groups.GetCount() - 1; i >= 0; i--) {
		const String& gpath = groups[i].vfs_path;
		if(node_id.StartsWith(gpath + "/") || node_id == gpath)
			return gpath;
	}
	return String();
}

// ---------------------------------------------------------------------------
// Walk the SemanticParser AST root and extract node type declarations + net
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
		if (child.GetName() != "node_type_decl")
			continue;

		NodeTypeDef& def = types.GetAdd(child.id.ToString());
		def.type_id = child.id.ToString();

		for (const AstNode& item : const_cast<AstNode&>(child).val.Sub<AstNode>()) {
			String name = item.GetName();
			if (name == "pin_in" || name == "pin_out") {
				// str = "portname|TYPE"
				Vector<String> parts = SplitPipe(item.str, 2);
				PinDef& p = def.pins.Add();
				p.name   = parts[0];
				p.type   = parts[1];
				p.is_out = (name == "pin_out");
			}
			else if (name == "node_param") {
				// str = "key|type|default|widget"
				Vector<String> parts = SplitPipe(item.str, 4);
				ParamDef& p = def.params.Add();
				p.name        = parts[0];
				p.type        = parts[1];
				p.default_val = parts[2];
				p.widget      = parts[3];
			}
		}
	}
}

// Map color name string to Color value
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

// Helper: parse integer from string, return false on failure
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

// Parse group style properties from string like "saturation: 50, hue: green, contrast: 50"
static GroupStyle ParseGroupStyle(const String& style_str)
{
	GroupStyle s;
	if(style_str.IsEmpty()) return s; // Return default style if empty
	
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
			val = TrimBoth(val);
			int v;
			if(ParseInt(val, v) && v >= 0 && v <= 100)
				s.saturation = v;
		}
		else if(key == "hue") {
			if(val == "red") s.hue = 0;
			else if(val == "orange") s.hue = 30;
			else if(val == "yellow") s.hue = 60;
			else if(val == "green") s.hue = 120;
			else if(val == "cyan") s.hue = 180;
			else if(val == "blue") s.hue = 240;
			else if(val == "purple") s.hue = 270;
			else if(val == "ltblue") s.hue = 200;
			else {
				val = TrimBoth(val);
				int v;
				if(ParseInt(val, v) && v >= 0 && v <= 360)
					s.hue = v;
			}
		}
		else if(key == "contrast") {
			val.Replace("%", "");
			val = TrimBoth(val);
			int v;
			if(ParseInt(val, v) && v >= 0 && v <= 100)
				s.contrast = v;
		}
	}
	return s;
}

// Map pin type name to display color
static Color PinTypeColor(const String& type_name)
{
	if (type_name == "MODEL")        return Color(175, 100, 255); // purple
	if (type_name == "CLIP")         return Color(255, 220,  50); // yellow
	if (type_name == "CONDITIONING") return Color(255, 150,  50); // orange
	if (type_name == "LATENT")       return Color(255, 180, 200); // pink
	if (type_name == "VAE")          return Color(220,  80,  80); // red
	if (type_name == "IMAGE")        return Color( 80, 210, 100); // green
	if (type_name == "INT")          return Color(150, 200, 255); // light blue
	if (type_name == "FLOAT")        return Color(200, 230, 180); // light green
	if (type_name == "STRING")       return Color(200, 200, 200); // gray
	if (type_name == "LAYER_STACK")  return Color(105, 185, 255); // neural stack
	if (type_name == "TENSOR")       return Color(150, 120, 255); // neural tensor
	if (type_name == "MODEL_STRING") return Color(205, 185, 255); // textual model
	if (type_name == "METRICS")      return Color(255, 165, 110); // metrics
	if (type_name == "REPORT")       return Color(235, 185, 115);
	if (type_name == "SCRIPT")       return Color(210, 210, 170);
	if (type_name == "HYPERPARAMETERS") return Color(180, 210, 150);
	return Color(160, 160, 160);
}

// Parse a single "key = value" assignment statement node
// Apply pin definitions from a NodeTypeDef to a NodeDoc
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

// Split "node_id.port_name" — tries to find the longest prefix that is a known node id.
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

// Extract the value string from a param ExprStmt's children.
// The AST for "key = value" is: Rval(key) + assign-op + const(value)
// We grab the last child's str (constant string) or id (identifier).
// AST layout for "key = value" in a net ExprStmt:
//   ExprStmt.rval → assign_op (Cursor_Op_ASSIGN)
//     assign_op.arg[0] → Rval node; Rval.rval → declared var with val.id = "key"
//     assign_op.arg[1] → Const node (str=string, i64=int, dbl=double)
static String ExtractParamValue(const AstNode& expr_stmt)
{
	if (!expr_stmt.rval) return String();
	const AstNode* assign_op = expr_stmt.rval;
	if (!assign_op->arg[1]) return String();
	const AstNode* val_node = assign_op->arg[1];
	// String literal
	if (!val_node->str.IsEmpty()) return val_node->str;
	// Integer / bool
	if (IsPartially(val_node->src, Cursor_Literal_INT32) ||
	    IsPartially(val_node->src, Cursor_Literal_BOOL))
		return IntStr((int)val_node->i64);
	// Double
	if (IsPartially(val_node->src, Cursor_Literal_DOUBLE))
		return DblStr(val_node->dbl);
	return String();
}

// Extract the param key name from a param ExprStmt.
// assign_op.arg[0] is the Rval; Rval.rval points to the declared variable whose val.id is the key.
static String ExtractParamKey(const AstNode& expr_stmt)
{
	if (!expr_stmt.rval) return String();
	const AstNode* assign_op = expr_stmt.rval;
	if (!assign_op->arg[0]) return String();
	const AstNode* rval = assign_op->arg[0];
	if (rval->rval) return rval->rval->GetName();
	return rval->GetName(); // fallback
}

// Walk the net CompoundStmt — atoms become nodes, ExprStmts become param
// assignments (associated with the last seen atom) or connections.
// AST layout (from SemanticParser output):
//   NetStmt: id
//     CompoundStmt:
//       VarDecl: param_name   (net-scope variable — skip)
//       AtomStmt: instance_id
//         CompoundStmt:
//           ExprStmt: Unresolved(type_path)   <- atom type
//       ExprStmt: Rval(key) assign const(val) <- param for last atom
//       ...
//       ExprStmt: str="src -> dst" obj=meta   <- connection
static void LoadNetCompound(const AstNode& compound, Graph& g,
                            const VectorMap<String, NodeTypeDef>& types,
                            Vector<ValidationMessage>& out)
{
	NodeDoc* current_atom = nullptr;
	int      current_type_idx = -1;

	for (const AstNode& stmt : const_cast<AstNode&>(compound).val.Sub<AstNode>()) {

		// Skip VarDecl (net-scope param name registrations)
		if (stmt.src == Cursor_VarDecl) continue;

		// AtomStmt: a new node instance
		if (stmt.src == Cursor_AtomStmt) {
			String atom_id = stmt.GetName();
			if (atom_id.IsEmpty()) {
				current_atom = nullptr;
				continue;
			}
			// Allow hierarchical IDs like /enc/node1, but strip leading slash for entity ID
			if(atom_id[0] == '/') atom_id = atom_id.Mid(1);
			if(!IsValidEntityId(atom_id)) {
				current_atom = nullptr;
				continue;
			}

			// Extract type path: atom → CompoundStmt → ExprStmt.rval → Unresolved.str
			String type_id = atom_id;
			for (const AstNode& achild : const_cast<AstNode&>(stmt).val.Sub<AstNode>()) {
				if (achild.src != Cursor_CompoundStmt) continue;
				for (const AstNode& gc : const_cast<AstNode&>(achild).val.Sub<AstNode>()) {
					if (gc.src != Cursor_ExprStmt) continue;
					const AstNode* unres = gc.rval;
					if (unres && unres->src == Cursor_Unresolved && !unres->str.IsEmpty()) {
						type_id = unres->str;
						break;
					}
				}
				if (type_id != atom_id) break;
			}

			current_atom = &g.AddNode(atom_id);
			current_atom->label = atom_id; // may be overridden by eon label

			// Store type metadata
			current_atom->node_type_id = type_id;
			{
				int dot = type_id.Find('.');
				current_atom->category = (dot >= 0) ? type_id.Left(dot) : type_id;
			}

			current_type_idx = types.Find(type_id);
			if (current_type_idx >= 0)
				ApplyPins(*current_atom, types[current_type_idx]);
			continue;
		}

		// ExprStmt: either a param assignment or a connection
		if (stmt.src == Cursor_ExprStmt) {
			// Connection: str contains " -> "
			const String& cs = stmt.str;
			if (cs.Find("->") >= 0) {
				int arrow = cs.Find("->");
				String src_full = TrimBoth(cs.Left(arrow));
				String dst_full = TrimBoth(cs.Mid(arrow + 2));

				String src_node, src_pin, dst_node, dst_pin;
				SplitNodePort(g, src_full, src_node, src_pin);
				SplitNodePort(g, dst_full, dst_node, dst_pin);
				if (src_node.IsEmpty() || dst_node.IsEmpty()) continue;

				// Build edge id — sanitize: replace dots with underscores
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
				current_atom = nullptr; // connections don't belong to an atom
				continue;
			}

			// Param assignment: belongs to current_atom
			if (!current_atom) continue;
			String key = ExtractParamKey(stmt);
			if (key.IsEmpty()) continue;
			String val = ExtractParamValue(stmt);

			// Special built-in instance properties that map to NodeDoc fields
			if (key == "label") {
				current_atom->label = val;
				continue;
			}

			WidgetSlotDoc& slot = current_atom->slots.Add();
			slot.id   = key;
			slot.type = "param";
			slot.properties.Add("value", val);
			if (current_type_idx >= 0) {
				const NodeTypeDef& ntd = types[current_type_idx];
				for (const ParamDef& pd : ntd.params) {
					if (pd.name == key) { slot.type = pd.widget.IsEmpty() ? "param" : pd.widget; break; }
				}
			}
			continue;
		}
	}
}

// Walk the net body
static void LoadNet(const AstNode& net_node, Graph& g,
                    const VectorMap<String, NodeTypeDef>& types,
                    Vector<ValidationMessage>& out)
{
	// NetStmt has one child: CompoundStmt
	for (const AstNode& child : const_cast<AstNode&>(net_node).val.Sub<AstNode>()) {
		if (child.src == Cursor_CompoundStmt)
			LoadNetCompound(child, g, types, out);
	}
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool LoadEon(Graph& g, const String& eon_text,
             Vector<ValidationMessage>& out)
{
	// First, extract group annotations from comments
	Vector<GroupDef> group_defs = ParseGroupAnnotations(eon_text);

#ifdef flagDEBUG
	LOG("Parsed " << group_defs.GetCount() << " groups from comments");
#endif

	VfsValue root_val;
	Compiler compiler(root_val);

	AstNode* sem_root = compiler.CompileToSemantic(eon_text, "<eon>", false);
	if (!sem_root) {
		out.Add(ValidationMessage(ValidationMessage::ERROR, "Eon parse failed"));
		return false;
	}

	// Collect node type declarations
	VectorMap<String, NodeTypeDef> types;
	CollectNodeTypeDefs(*sem_root, types);

	// Find and load net blocks — identified by src == Cursor_NetStmt
	bool found_net = false;
	for (const AstNode& top : sem_root->val.Sub<AstNode>()) {
		if (top.src == Cursor_NetStmt) {
			LoadNet(top, g, types, out);
			found_net = true;
		}
	}

	// Create groups from annotations and assign nodes based on path prefix
	for(int i = 0; i < group_defs.GetCount(); i++) {
		const GroupDef& gd = group_defs[i];
		String grp_id = gd.vfs_path.Mid(1); // Remove leading '/'
		if(grp_id.IsEmpty()) grp_id = "group_" + IntStr(i);
		GroupDoc& grp = g.AddGroup(grp_id);
		grp.label = gd.label;
		grp.vfs_path = gd.vfs_path;
		grp.style = gd.style;

		// Assign nodes whose IDs match this group's prefix
		// For example, group "/enc" matches nodes: enc_node1, enc_node80, etc.
		String prefix = gd.vfs_path.Mid(1); // Remove leading '/'
		for(auto& node : g.GetDoc().nodes) {
			if(node.id.StartsWith(prefix + "_") || node.id == prefix)
				grp.nodes.Add(node.id);
		}
#ifdef flagDEBUG
		LOG("Group " << grp_id << " has " << grp.nodes.GetCount() << " nodes");
#endif
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
	return LoadEon(g, content, out);  // Pass content directly
}

String SaveEon(const Graph& g)
{
	const GraphDoc& doc = g.GetDoc();
	String s;

	// Emit node type declarations for nodes that have pins
	Index<String> emitted_types;
	for (const NodeDoc& nd : doc.nodes) {
		if (nd.pins.IsEmpty() && nd.slots.IsEmpty()) continue;
		// Use label as type id if it looks like a dotted path, else skip type decl
		String type_id = nd.label;
		if (type_id.IsEmpty() || emitted_types.Find(type_id) >= 0) continue;
		emitted_types.Add(type_id);

		s << "node " << type_id << ":\n";
		for (const PinDoc& pin : nd.pins) {
			s << "\t" << (pin.kind == PinKind::Output ? "out" : "in")
			  << " " << pin.id;
			if(!pin.type_name.IsEmpty())
				s << " : " << pin.type_name;
			s << "\n";
		}
		for (WidgetSlotDoc& slot : const_cast<NodeDoc&>(nd).slots) {
			s << "\t" << slot.id;
			if (!slot.type.IsEmpty() && slot.type != "param")
				; // type hint not stored separately yet
			Value v = slot.properties.Get("value", Value());
			if (!v.IsVoid())
				s << " = " << v.ToString();
			s << "\n";
		}
		s << "\n";
	}

	// Emit net block
	s << "net graph:\n";

	for (const NodeDoc& nd : doc.nodes) {
		s << "\t" << nd.id;
		if (!nd.label.IsEmpty() && nd.label != nd.id)
			s << ": " << nd.label;
		bool has_slots = !nd.slots.IsEmpty();
		if (has_slots) {
			s << "\n";
			for (WidgetSlotDoc& slot : const_cast<NodeDoc&>(nd).slots) {
				Value v = slot.properties.Get("value", Value());
				s << "\t\t" << slot.id << " = ";
				if (!v.IsVoid()) s << v.ToString();
				s << "\n";
			}
		} else {
			s << "\n";
		}
	}
	s << "\n";

	// Connections
	for (const EdgeDoc& ed : doc.edges) {
		s << "\t" << ed.source_node;
		if (!ed.source_pin.IsEmpty()) s << "." << ed.source_pin;
		s << " -> " << ed.target_node;
		if (!ed.target_pin.IsEmpty()) s << "." << ed.target_pin;
		s << "\n";
	}

	return s;
}

} // namespace Node
} // namespace Upp
