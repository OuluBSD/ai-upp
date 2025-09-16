#include <Shell/Shell.h>

/*

loop tester.generator:
center.customer
center.audio.src.test
center.audio.sink.test.realtime:
	dbg_limit = 100

NamePart: tester
	LoopStmt: generator
		CompoundStmt:
			NamePart: center
				AtomStmt: customer
				NamePart: audio
					NamePart: src
						AtomStmt: test
					NamePart: sink
						NamePart: test
							AtomStmt: realtime
								CompoundStmt:
									ExprStmt:
										Unresolved: dbg_limit
											int32: const(int32: 100)
										assign: op(=)
*/

CONSOLE_APP_MAIN {
	using namespace Upp;
	Engine& eng = ShellMainEngine();
	eng.WhenUserInitialize << [](Engine& eng) {
		auto sys = eng.GetAdd<Eon::ScriptLoader>();
		if (0) {
			sys->PostLoadFile(GetDataFile("00a_audio_gen.eon"));
		}
		else {
			Eon::Builder& builder = sys->val.GetAdd<Eon::Builder>("builder");
			auto& loop = builder.AddLoop("tester.generator");
			auto& a0 = loop.AddAtom("center.customer");
			auto& a1 = loop.AddAtom("center.audio.src.test");
			auto& a2 = loop.AddAtom("center.audio.sink.test.realtime");
			a2.Assign("dbg_limit", 100);
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
	};
	
	ShellMain(true);
}
