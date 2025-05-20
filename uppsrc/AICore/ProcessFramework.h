#ifndef _AICore_ProcessFramework_h_
#define _AICore_ProcessFramework_h_

/*
TODO Read Notes:
	- fun dynamics:
		- add temperature to let agents ignore current AgentInteractionPolicy
			- or "not ignore", but change to secondary less-strict AgentInteractionPolicy
*/

class AgentInteractionPolicy;
class AgentInteractionSession;
class AgentInteractionSystem;



// Note: the Agent has additional Ecs::Component features,
//       but it's standalone without Ecs::Engine too.
class Agent : public Component {
	Vector<AgentInteractionSession*> sessions;
	ArrayMap<String, EscValue> global;
	int oplimit = 50000;
	hash_t compiled_hash = 0;
	
	bool Catch(Event<> cb, Vector<ProcMsg>& msgs);
	
public:
	CLASSTYPE(Agent)
	Agent(MetaNode& n);
	~Agent();
	
	using MsgCb = Event<Vector<ProcMsg>&>;
	
	void Visit(Vis& v) override {}
	bool RealizeLibrary(MsgCb WhenMessage);
	bool Compile(String esc, MsgCb WhenMessage=MsgCb());
	bool Run(MsgCb WhenMessage=MsgCb());
	
	void SetOpLimit(int i) {oplimit = i;}
	static int GetKind() {return METAKIND_ECS_COMPONENT_AI_AGENT;}
	
	Event<EscEscape&> WhenPrint;
	Event<EscEscape&> WhenInput;
	
};

INITIALIZE(Agent)
using AgentPtr = Ptr<Agent>;

class AgentInteractionSession : public MetaNodeExt {
	AgentPtr agent;
	
public:
	CLASSTYPE(AgentInteractionSession)
	AgentInteractionSession(MetaNode& n) : MetaNodeExt(n) {}
	
	void Visit(Vis& v) override {}
	
};

class AgentInteractionPolicy : public MetaNodeExt {
	enum {
		MODE_AGENT,    // Esc-script based essentially
		MODE_INTERNAL, // for high performance (without Esc script)
	};
	AgentPtr agent;
	
public:
	CLASSTYPE(AgentInteractionPolicy)
	AgentInteractionPolicy(MetaNode& n) : MetaNodeExt(n) {}
	
	void Visit(Vis& v) override {}
	
};

class AgentInteractionSystem : public MetaNodeExt {
	Vector<AgentPtr> agents;
	
	void Runner();
	RunningFlagSingle flag;
	
public:
	CLASSTYPE(AgentInteractionSystem)
	AgentInteractionSystem(MetaNode& n) : MetaNodeExt(n) {}
	
	void Visit(Vis& v) override {}
	void Start();
	void Stop();
	void Attach(Agent* a);
	void Detach(Agent* a);
	
	static void Setup();
	static void Uninstall();
	static AgentInteractionSystem* sys;
};


#endif
