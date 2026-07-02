#ifndef _dbg_Backend_h_
#define _dbg_Backend_h_

class IdeCoreWorkspace;
class IdeCoreConsoleHost;

struct DbgBackendInfo : Moveable<DbgBackendInfo> {
	String name;
	String description;
};

class DbgBackend {
public:
	virtual ~DbgBackend() {}

	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;

	virtual bool Initialize(IdeCoreWorkspace& workspace) { return true; }
	virtual bool Supports(const IdeCoreWorkspace& workspace) const { return true; }
	virtual int  Execute(IdeCoreConsoleHost& host, const char *cmdline) { return -1; }
	virtual int  Run(const Vector<String>& args) { return -1; }
};

Vector<DbgBackendInfo> GetPlannedDbgBackends();
const DbgBackendInfo *  FindPlannedDbgBackend(const String& name);
String                  GetPlannedDbgBackendList();
int                     RunDbgCli(const Vector<String>& args);

#endif
