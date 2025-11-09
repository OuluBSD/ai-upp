#include "Eon03.h"

/*
machine x11.app:

	driver context:
		x11.context

	chain program:

		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			x11.video.pipe

 NamePart: x11
	NamePart: app
		MachineStmt:
			DriverStmt: context
				CompoundStmt:
					NamePart: x11
						AtomStmt: context
			ChainStmt: program
				CompoundStmt:
					LoopStmt: video
						CompoundStmt:
							NamePart: center
								AtomStmt: customer
								NamePart: video
									NamePart: src
										AtomStmt: dbg_generator
											CompoundStmt:
												ExprStmt:
													Unresolved: mode
														string: const("noise")
													assign: op(=)
							NamePart: x11
								NamePart: video
									AtomStmt: pipe

*/

NAMESPACE_UPP

void Run03aX11Video(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 2: {
		using namespace Eon;
		// Manually writing directly to the VFS using Context
		Val& loop_root = eng.GetRootLoop();
		Val& space_root = eng.GetRootSpace();

		// Build VFS paths for machine.driver.chain.loop hierarchy
		VfsValue* l = &loop_root;
		VfsValue* s = &space_root;
		for (const String& part : Split("x11.app.context.program.video", ".")) {
			l = &l->GetAdd(part, 0);
			s = &s->GetAdd(part, 0);
		}

		// Alternative: resolve loop directly using Core helper
		VfsValue* l2 = Eon::ResolveLoopPath(eng, String("x11.app.context.program.video"));
		ASSERT(l2);
		if (l2 == l)
			LOG("ResolveLoopPath matched manual loop path");
		else
			LOG("ResolveLoopPath did not match manual loop path");

		ChainContext cc;

		// First, create the driver context atom: x11.context
		// This needs to be created BEFORE the video loop so it's in the VFS tree
		{
			Vector<ChainContext::AtomSpec> driver_atoms;
			ChainContext::AtomSpec& a = driver_atoms.Add();
			a.action = "x11.context";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);

			// Get driver context path in VFS
			VfsValue* driver_l = &loop_root;
			VfsValue* driver_s = &space_root;
			for (const String& part : Split("x11.app.context", ".")) {
				driver_l = &driver_l->GetAdd(part, 0);
				driver_s = &driver_s->GetAdd(part, 0);
			}

			// Create driver as a loop (it's a special loop that runs once)
			LoopContext& driver_loop = cc.AddLoop(*driver_l, driver_atoms, true);
			LOG("Driver context created:");
			LOG(driver_loop.GetTreeString(0));
		}

		// Now create the video loop with its atoms
		Vector<ChainContext::AtomSpec> atoms;

		// center.customer
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "center.customer";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}

		// center.video.src.dbg_generator with mode="noise"
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "center.video.src.dbg_generator";
			a.args.GetAdd("mode") = "noise";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}

		// x11.video.pipe
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "x11.video.pipe";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}

		LoopContext& loop = cc.AddLoop(*l, atoms, true);
		LOG(cc.GetTreeString(0));
		LOG(loop.GetTreeString(0));
		if (!cc.PostInitializeAll()) {
			LOG("PostInitialize failed");
			Exit(1);
		}
		if (!cc.StartAll()) {
			LOG("Start failed");
			cc.UndoAll();
			Exit(1);
		}
		break;
	}
	case 1: {
		// Manually building AstNode (skipping .eon file parsing)
		Eon::Builder& builder = sys->val.GetAdd<Eon::Builder>("builder");

		// Build machine/driver/chain/loop structure
		auto& machine = builder.AddMachine("x11.app");
		auto& driver = machine.AddDriver("context");
		driver.AddAtom("x11.context");

		auto& chain = machine.AddChain("program");
		auto& loop = chain.AddLoop("video");

		auto& a0 = loop.AddAtom("center.customer");
		auto& a1 = loop.AddAtom("center.video.src.dbg_generator");
		a1.Assign("mode", "noise");
		auto& a2 = loop.AddAtom("x11.video.pipe");

		Eon::AstNode* root = 0;
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
		break;
	}
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03a_x11_video.eon"));
		break;
	default:
		throw Exc(Format("Run03aX11Video: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
