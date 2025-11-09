#include "Eon02.h"

/*
loop player.audio.generator:
	center.customer
	center.audio.src.dbg_generator
	center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02aAudioTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 2: {
		// Manually writing directly to the VFS using Context
		using namespace Eon;
		// 1) Create loop + parallel space path: player.audio.generator
		Val& loop_root = eng.GetRootLoop();
		Val& space_root = eng.GetRootSpace();
		VfsValue* l = &loop_root;
		VfsValue* s = &space_root;
		for (const String& part : Split("player.audio.generator", ".")) {
			l = &l->GetAdd(part, 0);
			s = &s->GetAdd(part, 0);
		}
		// Alternative: resolve loop directly using Core helper
		VfsValue* l2 = Eon::ResolveLoopPath(eng, String("player.audio.generator"));
		ASSERT(l2);
		if (l2 == l)
			LOG("ResolveLoopPath matched manual loop path");
		else
			LOG("ResolveLoopPath did not match manual loop path");
		// 2) Describe atoms via actions and args
		ChainContext cc;
		Vector<ChainContext::AtomSpec> atoms;
		// center.customer
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "center.customer";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}
		// center.audio.src.dbg_generator
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "center.audio.src.dbg_generator";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}
		// center.audio.sink.hw
		{
			ChainContext::AtomSpec& a = atoms.Add();
			a.action = "center.audio.sink.hw";
			AtomTypeCls atom; LinkTypeCls link;
			if (ChainContext::ResolveAction(a.action, atom, link))
				a.iface.Realize(atom);
		}
		// 3) Build the loop under the loop path and link atoms
		LoopContext& loop = cc.AddLoop(*l, atoms, true);
		LOG(cc.GetTreeString(0));
		LOG(loop.GetTreeString(0));
		// 4) Finalize and start: call PostInitialize and Start via contexts
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
		auto& loop = builder.AddLoop("player.audio.generator");
		auto& a0 = loop.AddAtom("center.customer");
		auto& a1 = loop.AddAtom("center.audio.src.dbg_generator");
		auto& a2 = loop.AddAtom("center.audio.sink.hw");
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
		sys->PostLoadFile(ShareDirFile("eon/tests/02a_audio_test.eon"));
		break;
	default:
		throw Exc(Format("Run02aAudioTest: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
