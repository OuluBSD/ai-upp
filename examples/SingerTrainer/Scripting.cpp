#include "Scripting.h"
#include "SingerTrainer.h"
#include "ExerciseEngine.h"
#include <Core/TextParsing/TextParsing.h>

namespace Upp {

#ifdef flagGUI

GuiBridge::GuiBridge() {
	auto& dict = vm.GetGlobalsRW().GetDictRW();
	dict.GetAdd("FRY") = (int)FRY;
	dict.GetAdd("MODAL") = (int)MODAL;
	dict.GetAdd("COMPRESSED") = (int)COMPRESSED;
	dict.GetAdd("HEAD") = (int)HEAD;
	dict.GetAdd("FALSETTO") = (int)FALSETTO;
	dict.GetAdd("SUBHARMONIC") = (int)SUBHARMONIC;
	dict.GetAdd("DISTORTION") = (int)DISTORTION;
	
	dict.GetAdd("simulate_click") = PyValue::Function("simulate_click", PySimulateClick, this);
	dict.GetAdd("plotter_clear") = PyValue::Function("plotter_clear", PyPlotterClear, this);
	dict.GetAdd("plotter_add_node") = PyValue::Function("plotter_add_node", PyPlotterAddNode, this);
	dict.GetAdd("plotter_start") = PyValue::Function("plotter_start", PyPlotterStart, this);
}

void GuiBridge::RegisterCtrl(const String& name, Ctrl& ctrl) {
	ctrls.GetAdd(name) = &ctrl;
}

void GuiBridge::ExecuteScript(const String& code) {
	auto fn = [=] {
		Tokenizer tk;
		tk.SkipPythonComments();
		if(!tk.Process(code, "script")) return;
		tk.NewlineToEndStatement();
		tk.CombineTokens();

		PyCompiler compiler(tk.GetTokens());
		Vector<PyIR> ir;
		try {
			compiler.Compile(ir);
			vm.SetIR(ir);
			vm.Run();
		} catch (Exc& e) {
			LOG("Script error: " << e);
		}
	};

	if(IsMainThread())
		fn();
	else
		PostCallback(fn);
}

void GuiBridge::SimulateClick(const String& ctrlName) {
	int i = ctrls.Find(ctrlName);
	if (i >= 0) {
		Ctrl* ctrl = ctrls[i];
		if (ctrl) {
			ctrl->WhenAction();
		}
	}
}

PyValue GuiBridge::PySimulateClick(const Vector<PyValue>& args, void* user_data) {
	GuiBridge* bridge = (GuiBridge*)user_data;
	if (args.GetCount() > 0 && args[0].GetType() == PY_STR) {
		bridge->SimulateClick(args[0].GetStr().ToString());
	}
	return PyValue::None();
}

PyValue GuiBridge::PyPlotterClear(const Vector<PyValue>& args, void* user_data) {
	GuiBridge* bridge = (GuiBridge*)user_data;
	if (bridge->engine) bridge->engine->Clear();
	return PyValue::None();
}

PyValue GuiBridge::PyPlotterAddNode(const Vector<PyValue>& args, void* user_data) {
	GuiBridge* bridge = (GuiBridge*)user_data;
	if (bridge->engine && args.GetCount() >= 3) {
		bridge->engine->AddNode(args[0].AsDouble(), args[1].AsInt(), args[2].AsDouble());
	}
	return PyValue::None();
}

PyValue GuiBridge::PyPlotterStart(const Vector<PyValue>& args, void* user_data) {
	GuiBridge* bridge = (GuiBridge*)user_data;
	if (bridge->engine) bridge->engine->Start();
	return PyValue::None();
}

#endif

}
