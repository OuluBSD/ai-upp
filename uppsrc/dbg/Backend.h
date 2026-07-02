#ifndef _dbg_Backend_h_
#define _dbg_Backend_h_

class IdeCoreWorkspace;
class IdeCoreConsoleHost;

class DbgBackend {
public:
	virtual ~DbgBackend() {}

	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;

	virtual bool Initialize(IdeCoreWorkspace& workspace) { return true; }
	virtual bool Supports(const IdeCoreWorkspace& workspace) const { return true; }
	virtual int  Execute(IdeCoreConsoleHost& host, const char *cmdline) { return -1; }
};

VectorMap<String, String> GetPlannedDbgBackends();
String                    GetPlannedDbgBackendList();

#endif
