#ifndef _SingerTrainer_Scripting_h_
#define _SingerTrainer_Scripting_h_

#include <Core/Core.h>
#include <ByteVM/ByteVM.h>

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#endif

namespace Upp {

class ExerciseEngine;

#ifdef flagGUI
class GuiBridge {
	PyVM vm;
	VectorMap<String, Ctrl*> ctrls;
	ExerciseEngine* engine = nullptr;

public:
	GuiBridge();

	void SetEngine(ExerciseEngine& e) { engine = &e; }
	void RegisterCtrl(const String& name, Ctrl& ctrl);
	void ExecuteScript(const String& code);
	
	void SimulateClick(const String& ctrlName);

	PyVM& GetVM() { return vm; }
	
	static PyValue PySimulateClick(const Vector<PyValue>& args, void* user_data);
	static PyValue PyPlotterClear(const Vector<PyValue>& args, void* user_data);
	static PyValue PyPlotterAddNode(const Vector<PyValue>& args, void* user_data);
	static PyValue PyPlotterStart(const Vector<PyValue>& args, void* user_data);
};

#define BIND_ACTION(ctrl, python_func, bridge) \
	(ctrl).WhenAction = [this] { \
		this->bridge.ExecuteScript(String(python_func) + "()"); \
	};

#else
class GuiBridge {
public:
	void RegisterCtrl(const String& name, void* ctrl) {}
	void ExecuteScript(const String& code) {}
	void SimulateClick(const String& ctrlName) {}
	void SetEngine(void* e) {}
};

#define BIND_ACTION(ctrl, python_func, bridge)
#endif

}

#endif