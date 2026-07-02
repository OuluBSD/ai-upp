#ifndef _dbg_Session_h_
#define _dbg_Session_h_

struct DbgCallStackFrame : Moveable<DbgCallStackFrame> {
	String module;
	String function;
	String source_file;
	int    line = Null;
	String address;
};

struct DbgLaunchRequest : Moveable<DbgLaunchRequest> {
	String               executable_path;
	Vector<String>       arguments;
	String               working_directory;
	VectorMap<String, String> environment;
	bool                 quiet = true;
};

struct DbgRunResult : Moveable<DbgRunResult> {
	String               backend_name;
	bool                 started = false;
	bool                 crashed = false;
	int                  exit_code = Null;
	String               transcript;
	Vector<DbgCallStackFrame> call_stack;
	String               error;

	bool IsOk() const { return IsNull(exit_code) ? false : exit_code == 0 && !crashed && error.IsEmpty(); }
};

class DbgBackendSession {
public:
	virtual ~DbgBackendSession() {}

	virtual String       GetBackendName() const = 0;
	virtual DbgRunResult Run(const DbgLaunchRequest& request) = 0;
	virtual void         Cancel() {}
};

#endif
