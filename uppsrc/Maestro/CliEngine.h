#ifndef _Maestro_CliEngine_h_
#define _Maestro_CliEngine_h_

class CliMaestroEngine : public MaestroEngine {
	LocalProcess p;
	String       binary;
	Vector<String> args;
	String       buffer;
	
public:
	Function<void(const MaestroEvent&)> callback;

	CliMaestroEngine& Binary(const String& b) { binary = b; return *this; }
	CliMaestroEngine& Arg(const String& a)    { args.Add(a); return *this; }

	virtual void Send(const String& prompt, Function<void(const MaestroEvent&)> cb) override;
	virtual void Cancel() override;
	
	bool IsRunning() { return p.IsRunning(); }
	virtual bool Do() override;
};

#endif
