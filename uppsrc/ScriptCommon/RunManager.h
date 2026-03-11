#ifndef _ScriptCommon_RunManager_h_
#define _ScriptCommon_RunManager_h_

class RunManager {
public:
	RunManager(PyVM& vm);

	void Run(const String& code, const String& filename);
	void RunSelection(const String& code);
	void Stop();

	Event<> WhenStarted;
	Event<> WhenFinished;
	Event<const String&> WhenError;

private:
	PyVM& vm;
};

#endif
