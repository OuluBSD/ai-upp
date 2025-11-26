#include "Script.h"
// For Core contexts (AST-free building)
#include <Eon/Core/Core.h>

#define VERBOSE_SCRIPT_LOADER 0

NAMESPACE_UPP

extern void (*Eon_PostLoadString)(Engine& eng, String script_str);
extern bool (*Eon_AddScriptLoader)(Engine& eng);

namespace Eon {

void PostLoadString(Engine& eng, String script_str) {
	Ptr<Eon::ScriptLoader> script = eng.FindAdd<Eon::ScriptLoader>();
	if (script)
		script->PostLoadString(script_str);
}

bool AddScriptLoader(Engine& eng) {
	Ptr<Eon::ScriptLoader> script = eng.FindAdd<Eon::ScriptLoader>();
	if (!script) {
		LOG("No ScriptLoader added to machine and the machine is already started");
		return false;
	}
	
	#if 0
	#ifdef flagGUI
    Gu::GuboSystemPtr gubo	= mach.FindAdd<Gu::GuboSystem>();
    WindowSystemPtr win		= mach.FindAdd<WindowSystem>();
    #endif
    #endif
    return true;
}

INITBLOCK {
	Eon_PostLoadString = &PostLoadString;
	Eon_AddScriptLoader = &AddScriptLoader;
}


String Id::ToString() const {
	String s;
	for(const String& part : parts) {
		if (!s.IsEmpty())
			s << ".";
		s << part;
	}
	return s;
}

String Id::ToSlashPath() const {
	String s;
	for(const String& part : parts) {
		if (!s.IsEmpty())
			s << "/";
		s << part;
	}
	return s;
}

String NetConnectionDef::ToString() const {
	return Format("%s:%d -> %s:%d", from_atom, from_port, to_atom, to_port);
}

String NetConnectionDef::GetTreeString(int indent) const {
	String pad; pad.Cat(' ', indent);
	return pad + ToString();
}

String NetDefinition::GetTreeString(int indent) const {
	String s;
	String pad; pad.Cat(' ', indent);
	s << pad << "net " << id.ToString() << ":\n";
	for (const StateDeclaration& state : states)
		s << pad << "  state " << state.id.ToString() << "\n";
	for (const AtomDefinition& atom : atoms)
		s << pad << "  atom " << atom.id.ToString() << "\n";
	for (const NetConnectionDef& conn : connections)
		s << pad << "  " << conn.ToString() << "\n";
	for (const NetDefinition& subnet : subnets)
		s << subnet.GetTreeString(indent + 2);
	return s;
}

void NetDefinition::GetSubNetPointers(LinkedList<Eon::NetDefinition*>& ptrs) {
	for (NetDefinition& subnet : subnets) {
		ptrs.Add(&subnet);
		subnet.GetSubNetPointers(ptrs);
	}
}

static bool LooksLikeDotPath(const String& s) {
	if (s.IsEmpty())
		return false;
	if (s.Find('/') >= 0)
		return false;
	for (int i = 0; i < s.GetCount(); i++) {
		int chr = s[i];
		if (IsUpper(chr))
			return false;
	}
	int dot = s.Find('.');
	if (dot < 0 || dot == 0 || dot == s.GetCount() - 1)
		return false;
		
	// Check if this looks like a file with an extension (e.g., "filename.jpg")
	// In this case, don't treat it as a dot path
	Vector<String> parts = Split(s, ".");
	if (parts.GetCount() == 2) { // Only one dot - likely a filename.extension
		String first_part = parts[0];
		String second_part = parts[1];
		// If both parts are valid (alphanumeric, _, -) and the second part looks like an extension
		if (!first_part.IsEmpty() && !second_part.IsEmpty()) {
			// Check if first part is valid (alphanumeric, _, -)
			for (int i = 0; i < first_part.GetCount(); i++) {
				int chr = first_part[i];
				if (!IsAlNum(chr) && chr != '_' && chr != '-')
					return false;
			}
			// Check if second part is valid (alphanumeric, _, -)
			for (int i = 0; i < second_part.GetCount(); i++) {
				int chr = second_part[i];
				if (!IsAlNum(chr) && chr != '_' && chr != '-')
					return false;
			}
			
			// Common image extensions
			if (second_part == "jpg" || second_part == "jpeg" || second_part == "png" || 
				second_part == "gif" || second_part == "bmp" || second_part == "tga" || 
				second_part == "dds" || second_part == "psd" || second_part == "svg" || 
				second_part == "tif" || second_part == "tiff") {
				return false; // Don't treat filename.extension as a dotpath
			}
			
			// Common video extensions
			if (second_part == "mp4" || second_part == "avi" || second_part == "mov" || 
				second_part == "mkv" || second_part == "wmv" || second_part == "flv" || 
				second_part == "webm" || second_part == "m4v") {
				return false; // Don't treat filename.extension as a dotpath
			}
			
			// Common audio extensions
			if (second_part == "mp3" || second_part == "wav" || second_part == "ogg" || 
				second_part == "flac" || second_part == "aac" || second_part == "m4a") {
				return false; // Don't treat filename.extension as a dotpath
			}
			
			// Common text/data extensions
			if (second_part == "txt" || second_part == "json" || second_part == "xml" || 
				second_part == "csv" || second_part == "dat" || second_part == "bin") {
				return false; // Don't treat filename.extension as a dotpath
			}
		}
	}
	
	Vector<String> all_parts = Split(s, ".");
	if (all_parts.IsEmpty())
		return false;
	for (const String& part : all_parts) {
		if (part.IsEmpty())
			return false;
		for (int i = 0; i < part.GetCount(); i++) {
			int chr = part[i];
			if (!IsAlNum(chr) && chr != '_' && chr != '-')
				return false;
		}
	}
	return true;
}

static String NormalizeDotPath(const String& s) {
	return LooksLikeDotPath(s) ? Join(Split(s, "."), "/") : s;
}

static Vector<String> SplitLoopPath(const String& s) {
	if (s.Find('/') >= 0)
		return Split(s, "/");
	return Split(s, ".");
}

static void NormalizeValue(Value& v) {
	if (IsString(v)) {
		String str = v;
		String normalized = NormalizeDotPath(str);
		if (normalized != str)
			v = normalized;
	}
}






int ScriptLoader::loop_counter = 0;


ScriptLoader::ScriptLoader(VfsValue& n) :
	System(n),
	ErrorSource("ScriptLoader") {
	
}

ScriptLoader::~ScriptLoader() {
	
}

void ScriptLoader::Visit(Vis& v) {
	if (!loader.IsEmpty())
		v("loader",*loader,VISIT_NODE);
	
	_VIS_(tmp_side_id_counter)
	 VIS_(post_load_file)
	 VIS_(post_load_string)
	 VISN(cunit)
	 VIS_(collect_errors);
}

void ScriptLoader::LogMessage(ProcMsg msg) {
	LOG(msg.ToString());
}

bool ScriptLoader::Initialize(const WorldState& ws) {
	
	if (!WhenMessage)
		WhenMessage << THISBACK(LogMessage);
	
	GetEngine().AddUpdated(this);
	
	RTLOG("ScriptLoader::Initialize success!");
	return true;
}

bool ScriptLoader::PostInitialize() {
	if (!DoPostLoad())
		return false;
	
	return true;
}

bool ScriptLoader::DoPostLoad() {
	bool success = true;
	
	while (!post_load_file.IsEmpty()) {
		String path = post_load_file[0];
		post_load_file.Remove(0);
		
		if (!LoadFile(path))
			success = false;
	}
	
	while (!post_load_string.IsEmpty()) {
		String s = post_load_string[0];
		post_load_string.Remove(0);
		
		if (!Load(s, "input"))
			success = false;
	}
	
	return success;
}

void ScriptLoader::Update(double dt) {

	if (!DoPostLoad()) {
		GetEngine().SetNotRunning();
	}

	// Drive NetContext execution (Phase 5)
	for (One<NetContext>& nc : built_nets) {
		if (nc) {
			nc->Update(dt);
		}
	}

}

void ScriptLoader::Uninitialize() {
	//es.Clear();
	//ss.Clear();
	loader.Clear();
	GetEngine().RemoveUpdated(this);
}


PacketRouter* ScriptLoader::GetNetRouter(int net_idx) {
	if (net_idx < 0 || net_idx >= built_nets.GetCount())
		return nullptr;
	return built_nets[net_idx]->router.Get();
}


NetContext* ScriptLoader::GetNetContext(int net_idx) {
	if (net_idx < 0 || net_idx >= built_nets.GetCount())
		return nullptr;
	return built_nets[net_idx].Get();
}

int ScriptLoader::GetTotalPacketsRouted() const {
	int total = 0;
	for (const One<NetContext>& nc : built_nets)
		if (nc && nc->router)
			total += nc->router->GetTotalPacketsRouted();
	return total;
}

bool ScriptLoader::LoadFile(String path) {
	if (!FileExists(path)) {
		LOG("Could not find EON file");
		return false;
	}

	String eon_script;
	String ext = GetFileExt(path);
	if (ext == ".eon") {
		// Native .eon files are loaded directly
		eon_script = UPP::LoadFile(path);
	}
	else {
		// Use SerialLoaderFactory to handle different file types (.toy, .mid, etc.)
		eon_script = SerialLoaderFactory::LoadFile(path);
		if (eon_script.IsEmpty()) {
			LOG("ScriptLoader::LoadFile: no loader for extension: " << ext);
			return false;
		}
	}
	return Load(eon_script, path);
}

bool ScriptLoader::Load(const String& content, const String& filepath) {
	RTLOG("ScriptLoader::Load: Loading \"" << filepath << "\"");
	
	WhenEnterScriptLoad(*this);
	
	Compiler& c = val.GetAdd<Compiler>("cu");
	AstNode* root = c.CompileAst(content, filepath, true);
	
	if (!root) {
		RTLOG(GetLineNumStr(content, 1));
		WhenLeaveScriptLoad();
		return false;
	}
	return LoadAst(root);
}

bool ScriptLoader::LoadAst(AstNode* root) {
	if (!LoadCompilationUnit(root)) {
		LOG("error dump:");
		if (loader) loader->Dump();
		
		Cleanup();
		WhenLeaveScriptLoad();
		return false;
	}
	
	if (!ImplementScript()) {
		Cleanup();
		WhenLeaveScriptLoad();
		return false;
	}
	
	
	if (!loader->LoadEcs()) {
		AddError(root->loc, "ecs loading failed: " + loader->GetErrorString());
		return false;
	}
	
	
	
	Cleanup();
	WhenLeaveScriptLoad();
	
	return true;
}

static void EnsurePrefix(Eon::Id& id, const Eon::Id& prefix) {
	if (prefix.IsEmpty() || prefix.parts.IsEmpty())
		return;
	bool needs_prefix = true;
	if (id.parts.GetCount() >= prefix.parts.GetCount()) {
		needs_prefix = false;
		for (int i = 0; i < prefix.parts.GetCount(); i++) {
			if (id.parts[i] != prefix.parts[i]) {
				needs_prefix = true;
				break;
			}
		}
	}
	if (!needs_prefix)
		return;
	Vector<String> merged;
	merged <<= prefix.parts;
	merged.Append(id.parts);
	id.parts = pick(merged);
    RTLOG("EnsurePrefix: adjusted id -> " << id.ToString());
}

bool ScriptLoader::BuildChain(const Eon::ChainDefinition& chain) {
    One<ChainContext> cc = new ChainContext();
    RTLOG("BuildChain: chain=" << (chain.id.IsEmpty() ? "<anon>" : chain.id.ToString())
        << " loops=" << chain.loops.GetCount()
        << " states=" << chain.states.GetCount());

    // Pre-create state environments so downstream atoms can resolve targets during eager builds.
    Engine* mach = val.FindOwner<Engine>();
    ASSERT(mach);
    if (!mach)
        throw Exc("BuildChain: no engine available for state parent resolution");

    for (const Eon::StateDeclaration& state_def : chain.states) {
        if (state_def.id.IsEmpty())
            continue;

        const Vector<String>& parts = state_def.id.parts;
        if (parts.IsEmpty())
            continue;

        String state_leaf = parts.Top();
        Vector<String> parent_parts;
        parent_parts <<= parts;
        if (!parent_parts.IsEmpty())
            parent_parts.SetCount(parent_parts.GetCount() - 1);

        VfsValue* loop_parent = nullptr;
        VfsValue* space_parent = nullptr;
        if (parent_parts.IsEmpty()) {
            loop_parent = &mach->GetRootLoop();
            space_parent = &mach->GetRootSpace();
        }
        else {
            Eon::Id loop_id;
            loop_id.parts <<= parent_parts;
            loop_parent = ResolveLoop(loop_id, &space_parent);
            if (!loop_parent) {
                AddError(state_def.loc,
                         String("Could not resolve state parent loop: ") + loop_id.ToString());
                return false;
            }
        }

        if (!space_parent)
            space_parent = &mach->GetRootSpace();

        RTLOG("BuildChain: add EnvState parent="
            << (parent_parts.IsEmpty() ? String("<root>") : Join(parent_parts, "."))
            << " name=" << state_leaf);
        EnvState& env = loop_parent->GetAdd<EnvState>(state_leaf);
        space_parent->GetAdd(state_leaf, 0);
        env.SetName(state_leaf);
    }

    Vector<const Eon::LoopDefinition*> driver_loops;
    driver_loops.Reserve(chain.loops.GetCount());
    for (const Eon::LoopDefinition& loop_def : chain.loops)
        if (loop_def.is_driver)
            driver_loops.Add(&loop_def);

    // Create ordered list for initialization: drivers first, then regular loops
    Vector<const Eon::LoopDefinition*> init_order;
    init_order.Reserve(chain.loops.GetCount());
    for (const Eon::LoopDefinition& loop_def : chain.loops)
        if (loop_def.is_driver)
            init_order.Add(&loop_def);
    for (const Eon::LoopDefinition& loop_def : chain.loops)
        if (!loop_def.is_driver)
            init_order.Add(&loop_def);

    // Build each loop under its absolute id (loop.id is already resolved during parsing)
    // Process in initialization order: drivers first so they're available for regular loops
    for (const Eon::LoopDefinition* loop_def_ptr : init_order) {
        const Eon::LoopDefinition& loop_def = *loop_def_ptr;
        String chain_str = chain.id.IsEmpty() ? "<anon>" : chain.id.ToString();
        String loop_str = loop_def.id.IsEmpty() ? "<anon>" : loop_def.id.ToString();
        LOG(Format("BuildChain[%s]: loop=%s driver=%d atoms=%d",
                   chain_str, loop_str, (int)loop_def.is_driver, loop_def.atoms.GetCount()));
        RTLOG("\tloop=" << loop_def.id.ToString()
            << " driver=" << loop_def.is_driver
            << " atoms=" << loop_def.atoms.GetCount());
        Eon::Id deep_id = loop_def.id; // absolute id path
        if (!loop_def.is_driver && !driver_loops.IsEmpty()) {
            int best_idx = -1;
            int best_match = 0;
            for (int i = 0; i < driver_loops.GetCount(); i++) {
                const Eon::Id& drv = driver_loops[i]->id;
                int limit = min(drv.parts.GetCount(), deep_id.parts.GetCount());
                int match = 0;
                while (match < limit && drv.parts[match] == deep_id.parts[match])
                    match++;
                if (match > best_match) {
                    best_match = match;
                    best_idx = i;
                }
            }
            if (best_idx >= 0 && best_match > 0) {
                const Eon::Id& drv = driver_loops[best_idx]->id;
                // Only remap if driver is a complete prefix of the loop path
                // (all driver parts match the beginning of loop path)
                if (best_match == drv.parts.GetCount() && best_match < deep_id.parts.GetCount()) {
                    Vector<String> merged;
                    merged <<= drv.parts;
                    for (int i = best_match; i < deep_id.parts.GetCount(); i++)
                        merged.Add(deep_id.parts[i]);
                    bool changed = merged.GetCount() != deep_id.parts.GetCount();
                    if (!changed) {
                        for (int i = 0; i < merged.GetCount(); i++)
                            if (merged[i] != deep_id.parts[i]) { changed = true; break; }
                    }
                    if (changed) {
                        deep_id.parts = pick(merged);
                        LOG(Format("  remapped loop path under driver -> %s", deep_id.ToString()));
                    }
                }
            }
        }
        VfsValue* l = ResolveLoop(deep_id);
        if (!l) {
            AddError(loop_def.loc, String("Could not resolve loop id: ") + deep_id.ToString());
            return false;
        }
        RTLOG("\t resolved loop path=" << l->GetPath());

        Vector<ChainContext::AtomSpec> specs;
        bool has_link = !loop_def.is_driver;
        for (const Eon::AtomDefinition& a : loop_def.atoms) {
            ChainContext::AtomSpec& s = specs.Add();
            s.action = a.id.ToString();
            s.iface = a.iface;            // includes side-link conn field assignments
            s.link = has_link ? a.link : LinkTypeCls();
            s.args <<= a.args;           // copy args
        }

        LoopContext& lc = cc->AddLoop(*l, specs, has_link);
        if (lc.failed) {
            RTLOG("ScriptLoader::BuildChain: loop failed to initialize atoms");
            return false;
        }
    }

    // Connect side-links across loops in this chain according to conn ids
    for (int i = 0; i < cc->loops.GetCount(); i++)
        for (int j = 0; j < cc->loops.GetCount(); j++)
            if (i != j)
                if (!LoopContext::ConnectSides(cc->loops[i], cc->loops[j]))
                    return false;

    // Store the built chain context so ImplementScript can initialize and start it
    built_chains.Add(pick(cc));

    return true;
}

bool ScriptLoader::BuildNet(const Eon::NetDefinition& net) {
    RTLOG("BuildNet: net=" << (net.id.IsEmpty() ? "<anon>" : net.id.ToString())
        << " atoms=" << net.atoms.GetCount()
        << " states=" << net.states.GetCount()
        << " connections=" << net.connections.GetCount());

    // Resolve net space (similar to how BuildChain resolves loop space)
    Engine* mach = val.FindOwner<Engine>();
    ASSERT(mach);
    if (!mach) {
        AddError(FileLocation(), "BuildNet: no engine available");
        return false;
    }

    VfsValue* net_space = ResolveLoop(const_cast<Eon::Id&>(net.id));
    if (!net_space) {
        AddError(FileLocation(), String("Could not resolve net space: ") + net.id.ToString());
        return false;
    }

    // Create NetContext
    One<NetContext> nc = new NetContext(*net_space);
    LOG("BuildNet: Creating network for " << net.id.ToString());

    // Map atom names to their indices for connection resolution
    VectorMap<String, int> atom_index_map;

    // Create atoms
    int i = 0;
    for (const Eon::AtomDefinition& atom_def : net.atoms) {
        String atom_name = atom_def.id.ToString();
        String action = atom_name; // atom name is the action

        LOG("  Creating atom[" << i << "]: " << atom_name);

        AtomBasePtr atom = nc->AddAtom(atom_name, action, atom_def.iface, &atom_def.args);
        if (!atom) {
            AddError(atom_def.loc, "Failed to create atom: " + atom_name);
            return false;
        }

        atom_index_map.Add(atom_name, i);
        i++;
    }

    if (nc->failed) {
        AddError(FileLocation(), "BuildNet: atom creation failed for " + net.id.ToString());
        return false;
    }

    // Register ports
    if (!nc->RegisterPorts()) {
        AddError(FileLocation(), "BuildNet: port registration failed for " + net.id.ToString());
        return false;
    }

    // Add connections
    for (const NetConnectionDef& conn : net.connections) {
        int from_idx = atom_index_map.Find(conn.from_atom);
        int to_idx = atom_index_map.Find(conn.to_atom);

        if (from_idx < 0) {
            AddError(conn.loc, "Connection source atom not found: " + conn.from_atom);
            return false;
        }
        if (to_idx < 0) {
            AddError(conn.loc, "Connection sink atom not found: " + conn.to_atom);
            return false;
        }

        ValueMap conn_metadata;
        for (int i = 0; i < conn.metadata.GetCount(); i++)
            conn_metadata.Add(conn.metadata.GetKey(i), conn.metadata[i]);
        nc->AddConnection(from_idx, conn.from_port, to_idx, conn.to_port, conn_metadata);
        LOG("  Added connection: " << conn.ToString());
    }

    // Wire connections
    if (!nc->MakeConnections()) {
        AddError(FileLocation(), "BuildNet: connection wiring failed for " + net.id.ToString());
        return false;
    }

    // Store the built net context for later initialization in ImplementScript
    built_nets.Add(pick(nc));

    LOG("BuildNet: Successfully built network " << net.id.ToString()
        << " with " << built_nets.Top()->atoms.GetCount() << " atoms and "
        << built_nets.Top()->connections.GetCount() << " connections");

    return true;
}

void ScriptLoader::Cleanup() {
	loader.Clear();
}

bool ScriptLoader::ImplementScript() {
    RTLOG("ScriptLoader::ImplementScript: load states");
    Vector<ScriptStateLoader*> states;
    loader->GetStates(states);
    for (ScriptStateLoader* dl: states)
        if (!dl->Load())
            return false;

    if (!eager_build_chains || (built_chains.IsEmpty() && built_nets.IsEmpty())) {
        RTLOG("ScriptLoader::ImplementScript: build chains (loops)");
        if (!loader->Load())
            return false;

        // Collect loop pointers created by chain loaders for post steps
        Vector<ScriptLoopLoader*> loops;
        loader->GetLoops(loops);

        RTLOG("ScriptLoader::ImplementScript: connect sides");
        for (ScriptLoopLoader* loop0 : loops) {
            for (ScriptLoopLoader* loop1 : loops) {
                if (loop0 != loop1) {
                    if (!ConnectSides(*loop0, *loop1)) {
                        AddError(loop0->def.loc, "Side connecting failed");
                        return false;
                    }
                }
            }
        }

        RTLOG("ScriptLoader::ImplementScript: loop post initialize");
        for (ScriptLoopLoader* ll : loops) {
            if (!ll->PostInitialize())
                return false;
        }

        RTLOG("ScriptLoader::ImplementScript: loop start");
        for (ScriptLoopLoader* ll : loops) {
            if (!ll->Start())
                return false;
        }

        // Router nets built via BuildNet need the same lifecycle steps as chain contexts
        if (!built_nets.IsEmpty()) {
            RTLOG("ScriptLoader::ImplementScript: net post initialize");
            for (int i = 0; i < built_nets.GetCount(); i++) {
                NetContext& N = *built_nets[i];
                if (!N.PostInitializeAll()) {
                    AddError(FileLocation(), "Net PostInitialize failed");
                    for (int k = i; k >= 0; k--) built_nets[k]->UndoAll();
                    return false;
                }
            }

            RTLOG("ScriptLoader::ImplementScript: net start");
            for (int i = 0; i < built_nets.GetCount(); i++) {
                NetContext& N = *built_nets[i];
                if (!N.StartAll()) {
                    AddError(FileLocation(), "Net Start failed");
                    for (int k = i; k >= 0; k--) built_nets[k]->UndoAll();
                    return false;
                }
            }
        }
    } else {
        RTLOG("ScriptLoader::ImplementScript: eager mode: connect sides across built chains");
        for (int i = 0; i < built_chains.GetCount(); i++) {
            ChainContext& A = *built_chains[i];
            for (int j = 0; j < built_chains.GetCount(); j++) if (i != j) {
                ChainContext& B = *built_chains[j];
                for (auto& la : A.loops)
                    for (auto& lb : B.loops)
                        if (!LoopContext::ConnectSides(la, lb))
                            return false;
            }
        }

        RTLOG("ScriptLoader::ImplementScript: eager mode: validate side links");
        for (int i = 0; i < built_chains.GetCount(); i++) {
            String err_msg;
            if (!built_chains[i]->ValidateSideLinks(&err_msg)) {
                AddError(FileLocation(), String("Side-link validation failed: ") + err_msg);
                for (int k = i; k >= 0; k--) built_chains[k]->UndoAll();
                return false;
            }
        }

        RTLOG("ScriptLoader::ImplementScript: eager mode: post initialize chains");
        for (int i = 0; i < built_chains.GetCount(); i++) {
            ChainContext& C = *built_chains[i];
            if (!C.PostInitializeAll()) {
                for (int k = i - 1; k >= 0; k--) built_chains[k]->UndoAll();
                return false;
            }
        }

        RTLOG("ScriptLoader::ImplementScript: eager mode: post initialize nets");
        for (int i = 0; i < built_nets.GetCount(); i++) {
            NetContext& N = *built_nets[i];
            if (!N.PostInitializeAll()) {
                AddError(FileLocation(), "Net PostInitialize failed");
                for (int k = i; k >= 0; k--) built_nets[k]->UndoAll();
                return false;
            }
        }

        RTLOG("ScriptLoader::ImplementScript: eager mode: start chains");
        for (int i = 0; i < built_chains.GetCount(); i++) {
            ChainContext& C = *built_chains[i];
            if (!C.StartAll()) {
                for (int k = i; k >= 0; k--) built_chains[k]->UndoAll();
                return false;
            }
        }

        RTLOG("ScriptLoader::ImplementScript: eager mode: start nets");
        for (int i = 0; i < built_nets.GetCount(); i++) {
            NetContext& N = *built_nets[i];
            if (!N.StartAll()) {
                AddError(FileLocation(), "Net Start failed");
                for (int k = i; k >= 0; k--) built_nets[k]->UndoAll();
                return false;
            }
        }
    }
	
	return true;
}

bool ScriptLoader::GetPathId(Eon::Id& script_id, AstNode* from, AstNode* to) {
	Vector<Endpoint> path;
	
	if (from == to) {
		if (to->val.id.GetCount())
			path.Add(to);
	}
	else {
		AstNode* iter = to;
		while (iter && iter != from) {
			if (iter->val.id.GetCount() || iter == to) {
				path.Add(iter);
			}
			iter = iter->val.owner ? iter->val.owner->FindExt<AstNode>() : 0;
		}
	}
	if (path.IsEmpty()) {
		AddError(to->loc, "internal error: empty path");
		return false;
	}
	for (int i = path.GetCount()-1; i >= 0; i--) {
		AstNode* id = path[i].n;
		ASSERT(id->val.id.GetCount());
		script_id.parts.Add(id->val.id);
	}
	
	return true;
}

bool ScriptLoader::LoadCompilationUnit(AstNode* root) {
	loader.Clear();
	
	if (!LoadGlobalScope(cunit.glob, root))
		return false;
	
	loader = new ScriptSystemLoader(*this, 0, cunit.glob);
	
	return true;
}

bool ScriptLoader::LoadGlobalScope(Eon::GlobalScope& def, AstNode* n) {
	ASSERT(n);
	if (!n) return false;
	
	
	// Serial machine part
	Vector<Endpoint> items;
	n->FindAllNonIdEndpoints(items, Cursor_MetaDecl);
	Sort(items, AstNodeLess());
	
	bool has_machine = false;
	for (const Endpoint& ep : items) {
		AstNode* item = ep.n;
		if (item->src == Cursor_MachineDecl) {
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block"); return false;}
			
			Eon::MachineDefinition& mach_def = def.machs.Add();
			
			if (!GetPathId(mach_def.id, n, item))
				return false;
			
			ASSERT(!mach_def.id.IsEmpty());
			if (!LoadMachine(mach_def, block))
				return false;
			has_machine = true;
		}
		else if (item->src == Cursor_EngineStmt) {
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block for engine"); return false;}

			Eon::WorldDefinition& world_def = def.worlds.Add();

			if (!GetPathId(world_def.id, n, item))
				return false;

			ASSERT(!world_def.id.IsEmpty());
			if (!LoadWorld(world_def, block))
				return false;
		}
	}
	
	if (!has_machine) {
		Eon::MachineDefinition& mach = def.machs.Add();
		return LoadMachine(mach, n);
	}
	
	
	// Entity machine / ecs-engine part
	items.SetCount(0);
	n->FindAllNonIdEndpoints(items, Cursor_EcsStmt);
	Sort(items, AstNodeLess());
	
	bool has_world = false;
	for (const Endpoint& ep : items) {
		AstNode* item = ep.n;
		#if VERBOSE_SCRIPT_LOADER
		LOG(item->GetTreeString(0));
		#endif
		if (item->src == Cursor_WorldStmt) {
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block"); return false;}
			
			Eon::WorldDefinition& world_def = def.worlds.Add();
			
			if (!GetPathId(world_def.id, n, item))
				return false;
			
			ASSERT(!world_def.id.IsEmpty());
			if (!LoadWorld(world_def, block))
				return false;
			has_world = true;
		}
		else if (item->src == Cursor_SystemStmt) {
			// System statements are not allowed at the machine level, they should be inside worlds
			AddError(item->loc, "System statement is not allowed at machine level - systems belong in world definitions");
			return false;
		}
	}
	
	
	return true;
}

bool ScriptLoader::LoadMachine(Eon::MachineDefinition& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString());
	#endif

	Vector<Endpoint> items;
	n->FindAllNonIdEndpoints2(items, Cursor_EcsStmt, Cursor_OldEcsStmt);
	Sort(items, AstNodeLess());

	RTLOG("LoadMachine: found " << items.GetCount() << " items");
	for (int i = 0; i < items.GetCount(); i++) {
		RTLOG("  item[" << i << "]: src=" << GetCodeCursorString(items[i].n->src) << " id=" << items[i].n->val.id);
	}

	if (items.IsEmpty()) {
		AddError(def.loc, "empty node");
		return false;
	}
	
	Eon::ChainDefinition* anon_chain = 0;
	
	bool has_chain = false;
	for (const Endpoint& ep : items) {
		AstNode* item = ep.n;
		if (item->src == Cursor_MachineStmt) {
			// Nested machine - recursively process its contents
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block in nested machine"); return false;}

			// Update machine id to include the nested machine name
			Eon::Id nested_id;
			if (!GetPathId(nested_id, n, item))
				return false;
			EnsurePrefix(nested_id, def.id);

			// Temporarily update def.id for recursive processing
			Eon::Id saved_id = def.id;
			def.id = nested_id;

			if (!LoadMachine(def, block))
				return false;

			def.id = saved_id;
			has_chain = true;
		}
		else if (item->src == Cursor_ChainStmt) {
			Eon::ChainDefinition& chain_def = def.chains.Add();
			
			if (!GetPathId(chain_def.id, n, item))
				return false;
			EnsurePrefix(chain_def.id, def.id);
			
			ASSERT(!chain_def.id.IsEmpty());
			
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block"); return false;}
			
			Vector<Endpoint> items;
			block->FindAllNonIdEndpoints(items, Cursor_ChainStmt);
			if (items.IsEmpty()) {
				if (!LoadChain(chain_def, block))
					return false;
			} else {
				if (!LoadTopChain(chain_def, block))
					return false;
			}
			has_chain = true;
		}
		else if (item->src == Cursor_NetStmt) {
			Eon::NetDefinition& net_def = def.nets.Add();

			if (!GetPathId(net_def.id, n, item))
				return false;
			EnsurePrefix(net_def.id, def.id);

			ASSERT(!net_def.id.IsEmpty());

			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block"); return false;}

			if (!LoadNet(net_def, block))
				return false;
			has_chain = true;
		}
		else if (item->src == Cursor_DriverStmt || item->src == Cursor_LoopStmt) {
			if (!anon_chain)
				anon_chain = &def.chains.Add();
			if (anon_chain->id.IsEmpty())
				anon_chain->id = def.id;

			if (!LoadChain(*anon_chain, item))
				return false;
			has_chain = true;
		}
	}
	
	if (!has_chain) {
		Eon::ChainDefinition& chain = def.chains.Add();
		chain.id = def.id;
		return LoadChain(chain, n);
	}
	
	return true;
}

bool ScriptLoader::LoadWorld(Eon::WorldDefinition& def, AstNode* n) {
	Vector<Endpoint> items;
	n->FindAllNonIdEndpoints(items, Cursor_EcsStmt);
	Sort(items, AstNodeLess());
	
	if (items.IsEmpty()) {
		AddError(def.loc, "empty node");
		return false;
	}
	
	Eon::PoolDefinition* anon_pool = 0;
	
	bool has_chain = false;
	for (const Endpoint& ep : items) {
		AstNode* item = ep.n;
		if (item->src == Cursor_PoolStmt) {
			Eon::PoolDefinition& pool_def = def.pools.Add();
			
			if (!GetPathId(pool_def.id, n, item))
				return false;
			
			ASSERT(!pool_def.id.IsEmpty());
			
			AstNode* block = item->Find(Cursor_CompoundStmt);
			if (!block) {AddError(n->loc, "internal error: no stmt block"); return false;}
			
			Vector<Endpoint> items;
			block->FindAllNonIdEndpoints(items, Cursor_PoolStmt);
			if (items.IsEmpty()) {
				if (!LoadPool(pool_def, block))
					return false;
			} else {
				if (!LoadTopPool(pool_def, block))
					return false;
			}
			has_chain = true;
		}
		else if (item->src == Cursor_SystemStmt) {
			Eon::EcsSysDefinition& sys_def = def.systems.Add();
			
			if (!GetPathId(sys_def.id, n, item))
				return false;
			
			ASSERT(!sys_def.id.IsEmpty());
			
			if (!LoadEcsSystem(sys_def, item))
				return false;
			has_chain = true;
		}
	}
	
	return true;
}

bool ScriptLoader::LoadDriver(Eon::DriverDefinition& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString(0));
	#endif

	Vector<Endpoint> items;
	n->FindAll(items, Cursor_ArgumentStmt);
	Sort(items, AstNodeLess());

	for (Endpoint& ep : items) {
		AstNode* item = ep.n;
		Id arg_id;
		if (!GetId(arg_id, item)) return false;

		Value val;
		if (!GetValue(val, item)) return false;

		def.args.Add(arg_id, val);
	}

	return true;
}

bool ScriptLoader::LoadTopChain(Eon::ChainDefinition& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString(0));
	#endif

	return LoadChain(def, n);
}

bool ScriptLoader::LoadEcsSystem(Eon::EcsSysDefinition& def, AstNode* n) {
	
	if (!LoadArguments(def.args, n))
		return false;
	
	return true;
}

bool ScriptLoader::LoadPool(Eon::PoolDefinition& def, AstNode* n) {
	const auto& map = VfsValueExtFactory::AtomDataMap();
	Vector<Endpoint> items;
	
	n->FindAll(items, Cursor_EntityStmt);
	n->FindAll(items, Cursor_PoolStmt);
	Sort(items, AstNodeLess());
	
	for (Endpoint& ep : items) {
		AstNode* item = ep.n;
		if (item->src == Cursor_PoolStmt) {
			Eon::PoolDefinition& pool_def = def.pools.Add();
			pool_def.loc = item->loc;
			
			if (!GetPathId(pool_def.id, n, item))
				return false;
			
			if (!LoadPool(pool_def, item))
				return false;
		}
		else if (item->src == Cursor_EntityStmt) {
			Eon::EntityDefinition& ent_def = def.ents.Add();
			ent_def.loc = item->loc;
			
			if (!GetPathId(ent_def.id, n, item))
				return false;
			
			if (!LoadEntity(ent_def, item))
				return false;
		}
	}
	
	return true;
}

bool ScriptLoader::LoadTopPool(Eon::PoolDefinition& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString(0));
	#endif

	return LoadPool(def, n);
}

bool ScriptLoader::LoadState(Eon::StateDeclaration& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString(0));
	#endif

	// StateDeclaration has only id and location, which are typically set by the caller
	// This function ensures the state is properly initialized
	def.loc = n->loc;

	if (!GetPathId(def.id, n, n)) {
		return false;
	}

	return true;
}

bool ScriptLoader::LoadEntity(Eon::EntityDefinition& def, AstNode* n) {
	#if VERBOSE_SCRIPT_LOADER
	LOG(n->GetTreeString(0));
	#endif
	
	const auto& map = VfsValueExtFactory::AtomDataMap();
	Vector<Endpoint> items;
	
	n->FindAll(items, Cursor_ComponentStmt);
	Sort(items, AstNodeLess());
	
	for (Endpoint& ep : items) {
		AstNode* item = ep.n;
		Eon::ComponentDefinition& comp_def = def.comps.Add();
		comp_def.loc = item->loc;
		
		if (!GetPathId(comp_def.id, n, item))
			return false;
		
		if (!LoadComponent(comp_def, item))
			return false;
	}
	
	if (!LoadArguments(def.args, n))
		return false;
	
	return true;
}

bool ScriptLoader::LoadComponent(Eon::ComponentDefinition& def, AstNode* n) {
	
	if (!LoadArguments(def.args, n))
		return false;
	
	return true;
}

// Custom comparator to sort loops before drivers, then by location
struct LoopBeforeDriverLess {
	bool operator()(const Endpoint& a, const Endpoint& b) const {
		bool a_is_driver = a.n->src == Cursor_DriverStmt;
		bool b_is_driver = b.n->src == Cursor_DriverStmt;

		// If one is a driver and the other is a loop, loop comes first
		if (a_is_driver != b_is_driver)
			return !a_is_driver; // a comes before b if a is loop (false) and b is driver (true)

		// If both are the same type, sort by location
		return a.rel_loc < b.rel_loc;
	}
};

bool ScriptLoader::LoadChain(Eon::ChainDefinition& chain, AstNode* n) {
	const auto& map = VfsValueExtFactory::AtomDataMap();
	Vector<Endpoint> loops, states, atoms, stmts, conns;
	RTLOG("LoadChain: entering for id=" << chain.id.ToString() << " eager=" << eager_build_chains);

	n->FindAll(loops, Cursor_DriverStmt); // subset of loops
	n->FindAll(loops, Cursor_LoopStmt);
	Sort(loops, LoopBeforeDriverLess());
	
	for (Endpoint& ep : loops) {
		AstNode* loop = ep.n;
		bool is_driver = loop->src == Cursor_DriverStmt;
		
		Eon::Id loop_id;
		if (!GetPathId(loop_id, n, loop))
			return false;
		EnsurePrefix(loop_id, chain.id);

		AstNode* stmt_block = loop->Find(Cursor_CompoundStmt);
		if (!stmt_block) {
			AddError(loop->loc, "loop has no statement-block");
			return false;
		}
		
		atoms.SetCount(0);
		stmt_block->FindAll(atoms, Cursor_AtomStmt);
		Sort(atoms, AstNodeLess());
		
		if (atoms.IsEmpty()) {
			AddError(loop->loc, "no atoms in statement-block");
			return false;
		}
		
		Eon::Id single_atom_id;
		bool have_single_atom_id = false;
		
		if (is_driver && atoms.GetCount() > 1) {
			AddError(loop->loc, "only single atom is allowed in driver");
			return false;
		}
		
		if (atoms.GetCount() == 1) {
			if (!GetPathId(single_atom_id, loop, atoms[0].n))
				return false;
			have_single_atom_id = true;
			if (!is_driver) {
				String single_action = single_atom_id.ToString();
					if (single_action.StartsWith("state.")) {
						// Pure state loop; translate into a state declaration.
						Eon::StateDeclaration& state_def = chain.states.Add();
						state_def.loc = loop->loc;
						Eon::Id state_id = loop_id;
						Vector<String> derived_parts;
						for (int pi = 0; pi < single_atom_id.parts.GetCount(); ++pi) {
							const String& part = single_atom_id.parts[pi];
							if (pi == 0 && part == "state")
								continue;
							derived_parts.Add(part);
						}
						if (!derived_parts.IsEmpty()) {
							if (state_id.parts.IsEmpty() || state_id.parts.GetCount() < derived_parts.GetCount())
								state_id.parts <<= derived_parts;
						}
						EnsurePrefix(state_id, chain.id);
						state_def.id = state_id;
						continue;
					}
					AddError(loop->loc, "only one atom in the loop");
					return false;
				}
		}
		
		Eon::LoopDefinition& loop_def = chain.loops.Add();
		loop_def.loc = loop->loc;
		loop_def.is_driver = is_driver;
		loop_def.id = loop_id;

		for (int ai = 0; ai < atoms.GetCount(); ai++) {
			AstNode* atom = atoms[ai].n;
			Eon::AtomDefinition& atom_def = loop_def.atoms.Add();
			
			if (have_single_atom_id) {
				atom_def.id = single_atom_id;
				have_single_atom_id = false;
			}
			else {
				if (!GetPathId(atom_def.id, loop, atom))
					return false;
			}
			
			String loop_action = atom_def.id.ToString();
			const VfsValueExtFactory::AtomData* found_atom = 0;
			for (const VfsValueExtFactory::AtomData& atom_data : map.GetValues()) {
				bool match = false;
				for (const String& action : atom_data.actions) {
					if (action == loop_action) {
						match = true;
						break;
					}
				}
				if (!match)
					continue;
				found_atom = &atom_data;
			}
			
			if (!found_atom) {
				AddError(atom->loc, "could not find atom for '" + loop_action + "'");
				return false;
			}
			
			AtomTypeCls type = found_atom->cls;
			LinkTypeCls link = found_atom->link_type;
			ASSERT(type.IsValid());
			
			atom_def.iface.Realize(type);
			
			if (!is_driver) {
				ASSERT(atom_def.iface.sink.GetCount());
				ASSERT(atom_def.iface.src.GetCount());
				atom_def.link = link;
				
				conns.SetCount(0);
				atom->FindAllStmt(conns, Cursor_AtomConnectorStmt);
				Sort(conns, AstNodeLess());
				if (!conns.IsEmpty()) {
					int sink_conn_i = 0, src_conn_i = 0; // side ids start from 1, so this shouldn't be -1
					for (Endpoint& ep : conns) {
						AstNode* conn = ep.n;
						bool is_src = conn->i64 != 0;
						if (is_src)
							src_conn_i++;
						else
							sink_conn_i++;
						
						if (conn->val.Sub<AstNode>().IsEmpty()) {
							// Empty connector is allowed
							continue;
						}
							
						AstNode* expr = conn->Find(Cursor_ExprOp);
						if (!expr) {
							AddError(conn->loc, "internal error: no expression");
							return false;
						}
						
						bool succ = false;
						if (expr->src == Cursor_Op_EQ) {
							Eon::AtomDefinition::LinkCandidate& cand =
								is_src ?
									atom_def.src_link_cands.Add(src_conn_i) :
									atom_def.sink_link_cands.Add(sink_conn_i);
							
							AstNode* a0 = expr->arg[0];
							AstNode* a1 = expr->arg[1];
							while (a0->src == Cursor_Rval && a0->rval) a0 = a0->rval;
							while (a1->src == Cursor_Rval && a1->rval) a1 = a1->rval;
							String key;
							if (a0->src == Cursor_VarDecl) {
								key = a0->val.id;
							}
							else if (a0->src == Cursor_Unresolved) {
								key = a0->str;
							}
							if (key.GetCount()) {
								Value& req = cand.req_args.GetAdd(key);
								if (IsPartially(a1->src, Cursor_Literal)) {
									a1->CopyToValue(req);
									NormalizeValue(req);
									succ = true;
								}
								else if (a1->src == Cursor_Unresolved) {
									req = NormalizeDotPath(a1->str);
									succ = true;
								}
								else if (IsPartially(a1->src, Cursor_Op)) {
									req = EvaluateAstNodeValue(*a1);
									NormalizeValue(req);
									succ = true;
								}
							}
						}
						
						if (!succ) {
							AddError(expr->loc, "invalid atom connector expression");
							return false;
						}
					}
				}
			}
			
			if (!LoadArguments(atom_def.args, atom))
				return false;
		}
		
		if (!LoadArguments(loop_def.args, loop))
			return false;
		
	}
	RTLOG("LoadChain: parsed loops count=" << chain.loops.GetCount());
	
	for (Eon::LoopDefinition& src_loop : chain.loops) {
		for (Eon::AtomDefinition& src_atom : src_loop.atoms) {
			int src_count = src_atom.iface.type.iface.src.GetCount();
			
			for(int src_i = 1; src_i < src_count; src_i++) {
				const ValDevTuple::Channel& src_ch = src_atom.iface.type.iface.src[src_i];
				const ValDevCls& src_vd = src_ch.vd;
				bool src_is_opt = src_ch.is_opt;
				IfaceConnLink& src_conn = src_atom.iface.src[src_i];
				
				bool connected = false;
				
				int sink_cand_i = src_atom.src_link_cands.Find(src_i);
				Eon::AtomDefinition::LinkCandidate* src_cand =
					sink_cand_i >= 0 ?
						&src_atom.src_link_cands[sink_cand_i] : 0;
				
				ASSERT(src_conn.conn < 0);
				
				for (Eon::LoopDefinition& sink_loop : chain.loops) {
					if (&src_loop == &sink_loop)
						continue;
					for (Eon::AtomDefinition& sink_atom : sink_loop.atoms) {
						int sink_count = sink_atom.iface.type.iface.sink.GetCount();
						
						for(int sink_i = 1; sink_i < sink_count; sink_i++) {
							const ValDevTuple::Channel& sink_ch = sink_atom.iface.type.iface.sink[sink_i];
							const ValDevCls& sink_vd = sink_ch.vd;
							bool sink_is_opt = sink_ch.is_opt;
							IfaceConnLink& sink_conn = sink_atom.iface.sink[sink_i];
							
							int sink_cand_i = sink_atom.sink_link_cands.Find(sink_i);
							Eon::AtomDefinition::LinkCandidate* sink_cand =
								sink_cand_i >= 0 ?
									&sink_atom.sink_link_cands[sink_cand_i] : 0;
							
							// Only single fully optional is tried (so continue if connection exists)
							if ((sink_is_opt && !sink_cand) || (src_is_opt && !src_cand)) {
								if (src_atom.iface.HasCommonConnection(sink_atom.iface))
									continue;
							}
							
							if (sink_conn.conn < 0 && src_vd == sink_vd) {
								bool cond_prevents = false;
								if (src_cand) {
									for(int i = 0; i < src_cand->req_args.GetCount() && !cond_prevents; i++) {
										String key = src_cand->req_args.GetKey(i);
										if (key == "loop") {
											String loop_req = src_cand->req_args[i].ToString();
											Vector<String> parts = SplitLoopPath(loop_req);
											if (!sink_loop.IsPathTrailMatch(parts))
												cond_prevents = true;
										}
										else {
											int j = sink_loop.args.Find(key);
											if (j >= 0) {
												const Value& src_obj = src_cand->req_args[i];
												const Value& sink_obj = sink_loop.args[j];
												if (src_obj != sink_obj)
													cond_prevents = true;
											}
											else cond_prevents = true;
										}
									}
								}
								
								if (sink_cand) {
									for(int i = 0; i < sink_cand->req_args.GetCount() && !cond_prevents; i++) {
										String key = sink_cand->req_args.GetKey(i);
										if (key == "loop") {
											String loop_req = sink_cand->req_args[i].ToString();
											Vector<String> parts = SplitLoopPath(loop_req);
											if (!src_loop.IsPathTrailMatch(parts))
												cond_prevents = true;
										}
										else {
											int j = src_loop.args.Find(key);
											if (j >= 0) {
												const Value& sink_obj = sink_cand->req_args[i];
												const Value& src_obj = src_loop.args[j];
												if (sink_obj != src_obj)
													cond_prevents = true;
											}
											else cond_prevents = true;
										}
									}
								}
								
								if (!cond_prevents) {
									int conn_id = GetLoader().NewConnectionId();
									
									src_conn.conn = conn_id;
									src_conn.local = src_i;
									src_conn.other = sink_i;
									
									sink_conn.conn = conn_id;
									sink_conn.local = sink_i;
									sink_conn.other = src_i;
									
									connected = true;
									break;
								}
							}
						}
						
						if (connected)
							break;
					}
					
					if (connected)
						break;
				}
				
				if (!connected && !src_is_opt) {
					AddError(src_atom.loc, "could not connect source to any sink. Loop '" + src_loop.id.ToString() + "', atom '" + src_atom.id.ToString() + "'");
					return false;
				}
			}
		}
	}
	
	
	n->FindAll(states, Cursor_StateStmt);
	Sort(states, AstNodeLess());
	RTLOG("LoadChain: state stmt count=" << states.GetCount());
	for (Endpoint& ep : states) {
		AstNode* state = ep.n;
		Eon::StateDeclaration& state_def = chain.states.Add();
		state_def.loc = state->loc;
		
		if (!GetPathId(state_def.id, n, state))
			return false;
		RTLOG("LoadChain: state def=" << state_def.id.ToString());
	}
	
    bool ok = true;
    if (eager_build_chains) {
        RTLOG("LoadChain: eager build kick, loop defs before BuildChain=" << chain.loops.GetCount());
        // Experimental: directly materialize this chain using Core contexts.
        // Note: ImplementScript still builds via loader->Load(), so enabling this
        // can lead to duplicate instantiation. Keep it disabled unless using a
        // custom post-init/start path.
        if (!BuildChain(chain))
            ok = false;
        // Reduce memory: keep chain id/args, drop contents so loader tree stays small.
        chain.loops.Clear();
        chain.subchains.Clear();
        chain.states.Clear();
    }
    return ok;
}

bool ScriptLoader::LoadNet(Eon::NetDefinition& net, AstNode* n) {
	const auto& map = VfsValueExtFactory::AtomDataMap();
	Vector<Endpoint> atoms, states;
	RTLOG("LoadNet: entering for id=" << net.id.ToString());

	// Parse inline atom definitions
	n->FindAll(atoms, Cursor_AtomStmt);
	Sort(atoms, AstNodeLess());

	for (Endpoint& ep : atoms) {
		AstNode* atom = ep.n;
		Eon::AtomDefinition& atom_def = net.atoms.Add();
		atom_def.loc = atom->loc;

		if (!GetPathId(atom_def.id, n, atom))
			return false;

		String atom_action = atom_def.id.ToString();
		const VfsValueExtFactory::AtomData* found_atom = 0;
		for (const VfsValueExtFactory::AtomData& atom_data : map.GetValues()) {
			bool match = false;
			for (const String& action : atom_data.actions) {
				if (action == atom_action) {
					match = true;
					break;
				}
			}
			if (!match)
				continue;
			found_atom = &atom_data;
		}

		if (!found_atom) {
			AddError(atom->loc, "could not find atom for '" + atom_action + "'");
			return false;
		}

		AtomTypeCls type = found_atom->cls;
		ASSERT(type.IsValid());
		atom_def.iface.Realize(type);

		if (!LoadArguments(atom_def.args, atom))
			return false;
	}
	RTLOG("LoadNet: parsed atoms count=" << net.atoms.GetCount());

	// Parse state declarations
	n->FindAll(states, Cursor_StateStmt);
	Sort(states, AstNodeLess());
	RTLOG("LoadNet: state stmt count=" << states.GetCount());
	for (Endpoint& ep : states) {
		AstNode* state = ep.n;
		Eon::StateDeclaration& state_def = net.states.Add();
		state_def.loc = state->loc;

		if (!GetPathId(state_def.id, n, state))
			return false;
		RTLOG("LoadNet: state def=" << state_def.id.ToString());
	}

	// Parse explicit connections: atom:port -> atom:port
	// Connection syntax uses member access (.) for port references: atom.port -> atom.port
	Vector<Endpoint> conn_stmts;
	n->FindAll(conn_stmts, Cursor_ExprStmt);

	for (Endpoint& ep : conn_stmts) {
		AstNode* stmt = ep.n;
		if (!stmt->rval)
			continue;

		AstNode* rval = stmt->rval;
		while (rval->src == Cursor_Rval && rval->rval)
			rval = rval->rval;

		// Try to parse connection expression
		// We expect something like: osc.0 -> gain.0
		// This might parse as Cursor_Op_SUB (minus) followed by Cursor_Op_GT (greater than)
		// Or it might be a different operator structure
		// For now, use a simple string-based parser as fallback

		// Get the full expression text and parse it directly
		// Format: atom_name.port_id -> atom_name.port_id
		String expr_text;
		bool looks_like_connection = false;

		// Try to detect if this is a connection statement by checking for arrow-like patterns
		// or simple text extraction from the AST
		// First check if connection string is stored directly in stmt->str (from SemanticParser)
		if (!stmt->str.IsEmpty() && stmt->str.Find("->") >= 0) {
			expr_text = stmt->str;
			looks_like_connection = true;
		}
		else if (IsPartially(rval->src, Cursor_Op)) {
			// This is an operator expression - might be our connection
			// For now, we only handle simple Cursor_Unresolved connections
			// Full operator-based parsing will be implemented later
			RTLOG("LoadNet: skipping operator expression (not yet supported for connections)");
			continue;
		}
		else if (rval->src == Cursor_Unresolved) {
			// This might be a simple unresolved identifier that looks like a connection
			expr_text = rval->str;
			if (expr_text.Find("->") >= 0)
				looks_like_connection = true;
		}

		if (!looks_like_connection)
			continue;

		// Parse connection from text: "atom1.port1 -> atom2.port2" or "atom1:port1 -> atom2:port2"
		int arrow_pos = expr_text.Find("->");
		if (arrow_pos < 0)
			continue;

		String from_part = TrimBoth(expr_text.Left(arrow_pos));
		String to_part = TrimBoth(expr_text.Mid(arrow_pos + 2));

		// Parse from_part (atom.port or atom:port) - use LAST separator for port
		int from_sep = from_part.ReverseFind('.');
		if (from_sep < 0) from_sep = from_part.ReverseFind(':');
		if (from_sep < 0) {
			AddError(stmt->loc, "invalid connection syntax (missing port separator in source): " + expr_text);
			return false;
		}

		String from_atom = TrimBoth(from_part.Left(from_sep));
		String from_port_str = TrimBoth(from_part.Mid(from_sep + 1));

		// Parse to_part (atom.port or atom:port) - use LAST separator for port
		int to_sep = to_part.ReverseFind('.');
		if (to_sep < 0) to_sep = to_part.ReverseFind(':');
		if (to_sep < 0) {
			AddError(stmt->loc, "invalid connection syntax (missing port separator in sink): " + expr_text);
			return false;
		}

		String to_atom = TrimBoth(to_part.Left(to_sep));
		String to_port_str = TrimBoth(to_part.Mid(to_sep + 1));

		// Convert port strings to integers
		int from_port = 0;
		int to_port = 0;
		if (!from_port_str.IsEmpty() && IsDigit(from_port_str[0]))
			from_port = StrInt(from_port_str);
		if (!to_port_str.IsEmpty() && IsDigit(to_port_str[0]))
			to_port = StrInt(to_port_str);

		// Create connection definition
		NetConnectionDef& conn = net.connections.Add();
		conn.from_atom = from_atom;
		conn.from_port = from_port;
		conn.to_atom = to_atom;
		conn.to_port = to_port;
		conn.loc = stmt->loc;

		if (conn.metadata.GetCount() == 0) {
			conn.metadata.Add("policy", Value(String("legacy-loop")));
			conn.metadata.Add("credits", Value(1));
		}

		RTLOG("LoadNet: parsed connection: " << conn.ToString());
	}

	RTLOG("LoadNet: successfully parsed net with " << net.atoms.GetCount() << " atoms, "
		<< net.states.GetCount() << " states, " << net.connections.GetCount() << " connections");

	return true;
}

bool ScriptLoader::LoadArguments(ArrayMap<String, Value>& args, AstNode* n) {
	if (!n)
		return true; // only failed statements returns false
	
	if (n->src != Cursor_CompoundStmt) {
		AstNode* block = n->Find(Cursor_CompoundStmt);
		return LoadArguments(args, block);
	}
	
	AstNode* block = n;
	Vector<Endpoint> stmts;
	
	for (AstNode& stmt : block->val.Sub<AstNode>()) {
		if (!IsPartially(stmt.src, Cursor_Stmt) || stmt.src == Cursor_AtomConnectorStmt)
			continue;
		
		static int dbg_i;
		bool succ = false;
		if (stmt.src == Cursor_ExprStmt) {
			if (stmt.rval) {
				AstNode* r = stmt.rval;
				while (r->src == Cursor_Rval && r->rval) r = r->rval;
				AstNode& rval = *r;
				
				if (IsPartially(rval.src, Cursor_Op)) {
					if (rval.src == Cursor_Op_ASSIGN) {
						dbg_i++;
						AstNode* key = rval.arg[0];
						AstNode* value = rval.arg[1];
						while (key->src == Cursor_Rval && key->rval) key = key->rval;
						while (value->src == Cursor_Rval && value->rval) value = value->rval;
						if (key->src == Cursor_Unresolved && key->str.GetCount()) {
							String key_str = key->str;
							if (IsPartially(value->src, Cursor_Literal)) {
								Value val_obj;
								value->CopyToValue(val_obj);
								NormalizeValue(val_obj);
								args.GetAdd(key_str) = val_obj;
								succ = true;
							}
							else if (value->src == Cursor_Unresolved && value->str.GetCount()) {
								args.GetAdd(key_str) = NormalizeDotPath(value->str);
								succ = true;
							}
							else if (IsPartially(value->src, Cursor_Op)) {
								Value val_obj = EvaluateAstNodeValue(*value);
								NormalizeValue(val_obj);
								args.GetAdd(key_str) = val_obj;
								succ = true;
							}
							else {
								LOG(rval.GetTreeString(0));
								AddError(rval.loc, "Unsupported value type in argument assignment");
								return false;
							}
						}
						else if (key->src == Cursor_VarDecl) {
							String key_str = key->val.id;
							if (IsPartially(value->src, Cursor_Literal)) {
								Value val_obj;
								value->CopyToValue(val_obj);
								NormalizeValue(val_obj);
								args.GetAdd(key_str) = val_obj;
								succ = true;
							}
							else {
								LOG(rval.GetTreeString(0));
								AddError(rval.loc, "Unsupported value type in argument assignment");
								return false;
							}
						}
						else {
							LOG(rval.GetTreeString(0));
							AddError(rval.loc, "Unsupported syntax in arguments");
							return false;
						}
					}
					else {
						LOG(rval.GetTreeString(0));
						AddError(rval.loc, "Unsupported syntax in arguments");
						return false;
					}
				}
				else if (rval.src == Cursor_CompoundStmt ||
						 rval.src == Cursor_SymlinkStmt) {
					if (!LoadArguments(args, &rval))
						return false;
					succ = true;
				}
				else {
					LOG(rval.GetTreeString(0));
					AddError(rval.loc, "Unsupported syntax in arguments");
					return false;
				}
			}
			else {
				LOG(stmt.GetTreeString(0));
				AddError(stmt.loc, "Unsupported syntax in argument processing");
				return false;
			}
		}
		else {
			LOG(stmt.GetTreeString(0));
			AddError(stmt.loc, "Unsupported syntax in argument processing");
			return false;
		}
		
		if (!succ) {
			DUMP(dbg_i);
			LOG(stmt.GetTreeString(0));
			AddError(stmt.loc, "could not resolve statement in atom");
			return false;
		}
	}
	
	return true;
}

VfsValue* ScriptLoader::ResolveLoop(Eon::Id& id, VfsValue** space_out) {
	Engine* mach = val.FindOwner<Engine>();
	ASSERT(mach);
	if (!mach) throw Exc("no machine");
	
	VfsValue* l0 = &mach->GetRootLoop();
	VfsValue* s0 = &mach->GetRootSpace();
	if (id.parts.IsEmpty()) {
		if (space_out)
			*space_out = s0;
		return l0;
	}

	VfsValue* l1 = l0;
	VfsValue* s1 = s0;
	int i = 0, count = id.parts.GetCount();
	
	for (const String& part : id.parts) {
		if (i++ == count - 1) {
			l0 = &l1->GetAdd(part, 0);
			s0 = &s1->GetAdd(part, 0);
		}
		else {
			l1 = &l1->GetAdd(part, 0);
			s1 = &s1->GetAdd(part, 0);
		}
	}
	
	ASSERT(l0);
	if (space_out)
		*space_out = s0;
	return l0;
}

bool ScriptLoader::ConnectSides(ScriptLoopLoader& loop0, ScriptLoopLoader& loop1) {
	
	int dbg_i = 0;
	for (AtomBasePtr& sink : loop0.atoms) {
		LinkBasePtr sink_link = sink->GetLink();
		IfaceConnTuple& sink_iface = const_cast<IfaceConnTuple&>(sink->GetInterface());
		for (int sink_ch = 1; sink_ch < sink_iface.type.iface.sink.GetCount(); sink_ch++) {
			IfaceConnLink& sink_conn = sink_iface.sink[sink_ch];
			RTLOG("ScriptLoader::ConnectSides:	sink ch #" << sink_ch << " " << sink_conn.ToString());
			ASSERT(sink_conn.conn >= 0 || sink_iface.type.IsSinkChannelOptional(sink_ch));
			if (sink_conn.conn < 0 && sink_iface.type.IsSinkChannelOptional(sink_ch))
				continue;
			bool found = false;
			for (AtomBasePtr& src : loop1.atoms) {
				LinkBasePtr src_link = src->GetLink();
				IfaceConnTuple& src_iface = const_cast<IfaceConnTuple&>(src->GetInterface());
				for (int src_ch = 1; src_ch < src_iface.type.iface.src.GetCount(); src_ch++) {
					IfaceConnLink& src_conn = src_iface.src[src_ch];
					RTLOG("ScriptLoader::ConnectSides:		src ch #" << src_ch << " " << src_conn.ToString());
					ASSERT(src_conn.conn >= 0 || src_iface.type.IsSourceChannelOptional(src_ch));
					if (src_conn.conn < 0 && src_iface.type.IsSourceChannelOptional(src_ch))
						continue;
					if (sink_conn.conn == src_conn.conn) {
						RTLOG("ScriptLoader::ConnectSides:			linking side-link src ch #" << src_ch << " " << src_conn.ToString() << " to sink ch #" << sink_ch << " " << sink_conn.ToString());
						found = true;
						
						int src_ch_i = src_conn.local;
						int sink_ch_i = sink_conn.local;
						ASSERT(src_conn.other == sink_conn.local);
						ASSERT(sink_conn.other == src_conn.local);
						
						if (!src_link->LinkSideSink(sink_link, src_ch_i, sink_ch_i)) {
							AddError(loop0.def.loc, "Side-linking was refused");
							return false;
						}
						
						sink_conn.conn = -1;
						src_conn.conn = -1;
						
						#if VERBOSE_SCRIPT_LOADER
						LOG(ClassPathTop(src->ToString()) + "(" << HexStrPtr(&*src) << "," << src_ch_i << ") side-linked to " + ClassPathTop(sink->ToString()) + "(" << HexStrPtr(&*sink) << "," << sink_ch_i << ")");
						#else
						RTLOG(ClassPathTop(src->ToString()) + "(" << HexStrPtr(&*src) << "," << src_ch_i << ") side-linked to " + ClassPathTop(sink->ToString()) + "(" << HexStrPtr(&*sink) << "," << sink_ch_i << ")");
						#endif
						
						loop0.UpdateLoopLimits();
						loop1.UpdateLoopLimits();
						break;
					}
				}
			}
			
			
			dbg_i++;
		}
	}
	
	
	return true;
}


}
END_UPP_NAMESPACE
