#include "AICore.h"


NAMESPACE_UPP


Agent::Agent(MetaNode& n) : Component(n) {
	ASSERT(AgentInteractionSystem::sys);
	if (AgentInteractionSystem::sys) {
		AgentInteractionSystem::sys->Attach(this);
	}
}

Agent::~Agent() {
	if (AgentInteractionSystem::sys) {
		AgentInteractionSystem::sys->Detach(this);
	}
}

bool Agent::RealizeLibrary(Vector<ProcMsg>& msgs) {
	if (global.IsEmpty()) {
		Escape(global, "Print(x)", Proxy(WhenPrint));
		Escape(global, "Input()", Proxy(WhenInput));
		StdLib(global);
	}
	for(const FarStage& fs : stages) {
		for (const auto& fn : fs.funcs) {
			if (fn.esc_declaration.GetCount()) {
				Escape(global, fn.esc_declaration, THISBACK2(RunStage, fs.hash, fn.hash));
			}
		}
	}
	return true;
}

void Agent::RunStage(EscEscape& e, hash_t stage_hash, hash_t fn_hash) {
	const FarStage* stagep = 0;
	const FarStage::Function* fnp = 0;
	int fn_i = 0;
	for (const FarStage& s : stages) {
		if (s.hash != stage_hash) continue;
		stagep = &s;
		for (const auto& fn : s.funcs) {
			if (fn.hash != fn_hash) {fn_i++; continue;}
			fnp = &fn;
			break;
		}
		break;
	}
	if (!stagep || !fnp)
		e.esc.ThrowError("Stage not found");
	TODO // check params
	/*ValueMap params;
	TaskMgr& m = AiTaskManager();
	m.GetFarStage(stagep, fn_i, params);
	TODO*/
}

bool Agent::Catch(Event<> cb, Vector<ProcMsg>& msgs) {
	bool succ = true;
	try {
		cb();
	}
	catch(CParser::Error e) {
		ProcMsg& m = msgs.Add();
		m.msg = e;
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	catch(Exc e) {
		ProcMsg& m = msgs.Add();
		m.msg = "Exception: " + e;
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	catch(...) {
		ProcMsg& m = msgs.Add();
		m.msg = "Unknown error";
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	return succ;
}

bool Agent::CompileStage(MetaNode& stage, MsgCb WhenMessage) {
	TimeStop ts;
	
	ASSERT(stage.kind == METAKIND_ECS_COMPONENT_AI_STAGE);
	if (stage.kind != METAKIND_ECS_COMPONENT_AI_STAGE)
		return false;
	
	bool succ = true;
	FarStageCompiler comp;
	
	Vector<ProcMsg> msgs;
	succ = Catch([this,&comp,&stage]{
		comp.Compile(stage);
	}, msgs);
	
	msgs.Append(comp.GetMessages());
	
	if (succ) {
		ProcMsg& m = msgs.Add();
		m.msg = "Stage compilation succeeded in " + ts.ToString();
		m.severity = PROCMSG_INFO;
		
		stages.Add(comp.PopResult());
	}
	
	if (msgs.GetCount())
		WhenMessage(msgs);
	
	return succ;
}

bool Agent::Compile(String esc, MsgCb WhenMessage) {
	TimeStop ts;
	
	hash_t h = esc.GetHashValue();
	if (compiled_hash && compiled_hash == h)
		return true;
	
	compiled_hash = 0;
	bool succ = true;
	
	Vector<ProcMsg> msgs;
	
	global.Clear(); // clear global for now, because of stage functions
	
	if (!RealizeLibrary(msgs)) {
		WhenMessage(msgs);
		return false;
	}
	
	succ = Catch([this,&esc]{
		Scan(global, esc);
	}, msgs);
	
	if (succ)
		succ = CompileLambdas(msgs, WhenMessage);
	
	if (succ) {
		ProcMsg& m = msgs.Add();
		m.msg = "Compile succeeded in " + ts.ToString();
		m.severity = PROCMSG_INFO;
	}
	
	if (msgs.GetCount())
		WhenMessage(msgs);
	
	if (succ)
		compiled_hash = h;
	
	return succ;
}

bool Agent::CompileLambdas(Vector<ProcMsg>& msgs, MsgCb WhenMessage) {
	bool succ = true;
	#if USE_ESC_BYTECODE
	for(int i = 0; i < global.GetCount(); i++) {
		String key = global.GetKey(i);
		EscValue& ev = global[i];
		if (!ev.IsLambda())
			continue;
		EscLambda& l = ev.GetLambdaRW();
		if (l.compiled || l.escape)
			continue;
		
		if (!Catch([this,&ev]{
			::Upp::Compile(global, 0, ev);
		}, msgs))
			succ = false;
	}
	#endif
	return succ;
}

bool Agent::Run(MsgCb WhenMessage) {
	bool succ = true;
	
	Vector<ProcMsg> msgs;
	succ = Catch([this, &msgs]{
		Execute(global, "main", oplimit, [&msgs](ProcMsg& m) {msgs.Add(m);});
	}, msgs);
	
	if (msgs.GetCount())
		WhenMessage(msgs);
	
	return succ;
}

INITIALIZER_COMPONENT(Agent);



void AgentInteractionSystem::Start() {
	if (flag.IsRunning()) return;
	flag.Start();
	Thread::Start(THISBACK(Runner));
}

void AgentInteractionSystem::Stop() {
	flag.Stop();
}

void AgentInteractionSystem::Runner() {
	
	while (flag.IsRunning() && !Thread::IsShutdownThreads()) {
		
		
		Sleep(1);
	}
	
	flag.SetStopped();
}

void AgentInteractionSystem::Attach(Agent* a) {
	for (auto& ap : agents)
		if (ap == a)
			return;
	
}

void AgentInteractionSystem::Detach(Agent* a) {
	for(int i = 0; i < agents.GetCount(); i++) {
		if (agents[i] == a) {
			agents.Remove(i);
			
			
			return;
		}
	}
}

void AgentInteractionSystem::Setup() {
	AgentInteractionSystem& sys = MetaEnv().root.Add<AgentInteractionSystem>("agentsys");
	AgentInteractionSystem::sys = &sys;
	sys.Start();
}

void AgentInteractionSystem::Uninstall() {
	if (sys)
		sys->Stop();
}

AgentInteractionSystem* AgentInteractionSystem::sys;


END_UPP_NAMESPACE
