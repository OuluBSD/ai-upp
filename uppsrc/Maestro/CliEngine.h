#ifndef _Maestro_CliEngine_h_
#define _Maestro_CliEngine_h_

class CliMaestroEngine : public MaestroEngine {
	One<LocalProcess> p;
	String       buffer;
	
public:
	String       binary;
	Vector<String> args;
	String       working_dir;
	
	Function<void(const MaestroEvent&)> callback;

	CliMaestroEngine& Binary(const String& b) { binary = b; return *this; }
	CliMaestroEngine& Arg(const String& a)    { args.Add(a); return *this; }
	void             Reset()                  { binary.Clear(); args.Clear(); }

	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) override;
	virtual void Cancel() override;
	
	bool IsRunning() { return p && p->IsRunning(); }
	virtual bool Do() override;
	
	void WriteToolResult(const String& tool_id, const Value& result);
	
	virtual void ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) override;
	
	VectorMap<String, Array<SessionInfo>> project_sessions;
};

#endif