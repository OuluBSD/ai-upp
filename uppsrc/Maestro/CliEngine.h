
#ifndef _Maestro_CliEngine_h_
#define _Maestro_CliEngine_h_


class CliMaestroEngine : public MaestroEngine {
	One<LocalProcess> p;
	String       buffer;
	
public:
	String       binary;
	Vector<String> args;
	String       working_dir;
	
	Function<void(const MaestroEvent&)> event_callback;

	CliMaestroEngine& Binary(const String& b) { binary = b; return *this; }
	CliMaestroEngine& Arg(const String& a)    { args.Add(a); return *this; }
	void             Reset()                  { binary.Clear(); args.Clear(); }
	
	virtual ~CliMaestroEngine();

	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) override;
	virtual void Cancel() override;
	
	bool IsRunning() { return p && p->IsRunning(); }
	virtual bool Do() override;
	
	void WriteToolResult(const String& tool_id, const Value& result);
	
	virtual void ListSessions(const String& cwd, Function<void(const Array<SessionInfo>&)> cb) override;
	
	VectorMap<String, Array<SessionInfo>> project_sessions;
	
	// Gemini Cache
	struct GeminiSessionCache {
		Vector<SessionInfo> sessions;
		Time               last_update = Time::Low();
		
		void Jsonize(JsonIO& io) {
			io("sessions", sessions)("last_update", last_update);
		}
	};
	
	static GeminiSessionCache gemini_cache;
	static Mutex              gemini_mutex;
	static bool               gemini_updating;
	static Thread             update_thread;
	
	static void UpdateGeminiSessions();
	static void LoadGeminiCache();
	static void SaveGeminiCache();
	
	typedef CliMaestroEngine CLASSNAME;
};


#endif

