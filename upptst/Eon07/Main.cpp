#include <Shell/Shell.h>

/*
machine ecs.app:
    pass

world ecs.dummy:
    system rendering:
        dummy = true
    system events
    system physics:
        //log: debug
    
    pool world:
        entity ground:
            comp transform3
            comp model:
                builtin = plane
            
            comp physics:
                bind = physics
                test.fn = fixed
        
        entity ball:
            comp transform3
            comp body
            comp model:
                builtin = sphere
            
            comp physics:
                bind = physics
                test.fn = do.circle
        
        entity player:
            comp transform3:
                x = 0
                y = 0
                z = 6
            comp viewable
            comp viewport
            comp camera.chase:
                target = world.ball
                log = test
            comp physics:
                bind = physics
                test.fn = fixed

NamePart: ecs
	AtomStmt: app
	NamePart: ecs
	NamePart: dummy
	AtomStmt: world
	CompoundStmt:
		NamePart: rendering
			NamePart: system
			CompoundStmt:
				ExprStmt:
					Unresolved: dummy
					Bool: const(true)
					assign: op(=)
		NamePart: events
			NamePart: system
		NamePart: physics
			NamePart: system
			CompoundStmt:
				Comment: log: debug
*/

CONSOLE_APP_MAIN {
	using namespace Upp;
	Engine& eng = ShellMainEngine();
	eng.WhenUserInitialize << [](Engine& eng) {
		auto sys = eng.GetAdd<Eon::ScriptLoader>();
		sys->SetEagerChainBuild(true);
		if (1) {
			// Manually writing directly to the VFS using Context
			using namespace Eon;
			
			// Handle machine ecs.app
			{
				Val& machine_loop = eng.GetRootLoop().GetAdd("app", 0).GetAdd("ecs", 0);
				ChainContext cc_machine;
				Vector<ChainContext::AtomSpec> machine_atoms;
				ChainContext::AtomSpec& a = machine_atoms.Add();
				a.action = "ecs.app";
				AtomTypeCls atom; LinkTypeCls link;
				if (ChainContext::ResolveAction(a.action, atom, link))
					a.iface.Realize(atom);
				LoopContext& machine_loop_ctx = cc_machine.AddLoop(machine_loop, machine_atoms, true);
				LOG("Machine context: " << cc_machine.GetTreeString(0));
				LOG("Machine loop: " << machine_loop_ctx.GetTreeString(0));
				if (!cc_machine.PostInitializeAll()) {
					LOG("Machine PostInitialize failed");
				}
				if (!cc_machine.StartAll()) {
					LOG("Machine Start failed");
				}
			}
			
			// Create the world ecs.dummy
			Val& loop_root = eng.GetRootLoop();
			Val& space_root = eng.GetRootSpace();
			VfsValue* l = &loop_root;
			VfsValue* s = &space_root;
			for (const String& part : Split("ecs.dummy", ".")) {
				l = &l->GetAdd(part, 0);
				s = &s->GetAdd(part, 0);
			}
			// Alternative: resolve loop directly using Core helper
			VfsValue* l2 = Eon::ResolveLoopPath(eng, String("ecs.dummy"));
			ASSERT(l2);
			if (l2 == l)
				LOG("ResolveLoopPath matched world path");
			else
				LOG("ResolveLoopPath did not match world path");
			// Describe ECS components in the world via actions and args
			ChainContext cc;
			Vector<ChainContext::AtomSpec> atoms;
			// ecs.dummy world
			{
				ChainContext::AtomSpec& a = atoms.Add();
				a.action = "ecs.dummy";
				AtomTypeCls atom; LinkTypeCls link;
				if (ChainContext::ResolveAction(a.action, atom, link))
					a.iface.Realize(atom);
			}
			// system rendering with dummy=true
			{
				ChainContext::AtomSpec& a = atoms.Add();
				a.action = "system.rendering";
				a.args.GetAdd("dummy") = true;
				AtomTypeCls atom; LinkTypeCls link;
				if (ChainContext::ResolveAction(a.action, atom, link))
					a.iface.Realize(atom);
			}
			// system events
			{
				ChainContext::AtomSpec& a = atoms.Add();
				a.action = "system.events";
				AtomTypeCls atom; LinkTypeCls link;
				if (ChainContext::ResolveAction(a.action, atom, link))
					a.iface.Realize(atom);
			}
			// system physics
			{
				ChainContext::AtomSpec& a = atoms.Add();
				a.action = "system.physics";
				AtomTypeCls atom; LinkTypeCls link;
				if (ChainContext::ResolveAction(a.action, atom, link))
					a.iface.Realize(atom);
			}
			// Build the world under the loop path and link atoms
			LoopContext& loop = cc.AddLoop(*l, atoms, true);
			LOG(cc.GetTreeString(0));
			LOG(loop.GetTreeString(0));
			// Finalize and start: call PostInitialize and Start via contexts
			if (!cc.PostInitializeAll()) {
				LOG("PostInitialize failed");
				Exit(1);
			}
			if (!cc.StartAll()) {
				LOG("Start failed");
				cc.UndoAll();
				Exit(1);
			}
		}
		else if (1) {
			// Manually building AstNode (skipping .eon file parsing)
			Eon::Builder& builder = sys->val.GetAdd<Eon::Builder>("builder");
			
			// Create machine
			auto& machine_loop = builder.AddLoop("app.ecs");
			auto& machine_atom = machine_loop.AddAtom("ecs.app");
			
			// Create the world ecs.dummy
			auto& world_loop = builder.AddLoop("ecs.dummy");
			auto& w0 = world_loop.AddAtom("ecs.dummy");
			auto& w1 = world_loop.AddAtom("system.rendering");
			w1.Assign("dummy", true);
			auto& w2 = world_loop.AddAtom("system.events");
			auto& w3 = world_loop.AddAtom("system.physics");
			
			AstNode* root = 0;
			try {
				root = builder.CompileAst();
				if (!root) throw Exc("empty root");
			}
			catch (Exc e) {
				LOG("error: " << e);
				Exit(1);
			}
			LOG(root->GetTreeString());
			sys->LoadAst(root);
		}
		else {
			// Load eon file (parses AstNode and loads it)
			sys->PostLoadFile(GetDataFile("07a_ecs_dummy.eon"));
		}
	};
	
	ShellMain(true);
}
