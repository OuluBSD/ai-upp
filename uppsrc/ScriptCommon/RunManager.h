#ifndef _ScriptCommon_RunManager_h_
#define _ScriptCommon_RunManager_h_

class RunManager {
public:
	enum Mode {
		RUN_NORMAL,
		RUN_DEBUG,
		RUN_PROFILE,
	};

	RunManager(PyVM& vm);

	void SetMode(Mode mode) { this->mode = mode; }
	Mode GetMode() const { return mode; }
	void Run(const String& code, const String& filename);
	void RunSelection(const String& code);
	void Stop();

	Event<> WhenStarted;
	Event<> WhenFinished;
	Event<const String&> WhenError;

private:
	PyVM& vm;
	Mode mode = RUN_NORMAL;
};

#endif
