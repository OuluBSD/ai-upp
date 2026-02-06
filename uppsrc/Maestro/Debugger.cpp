#include "Maestro.h"

namespace Upp {

GdbService::GdbService() {
}

void GdbService::Run(const String& cmd, const String& args) {
	if(WhenOutput) WhenOutput("Starting GDB on " + cmd + " " + args + "...\n");
	if(WhenRunning) WhenRunning();
	// Stub: Simulate hitting a breakpoint
	PostCallback([=] {
		if(WhenOutput) WhenOutput("Breakpoint hit at main.cpp:42\n");
		if(WhenPaused) WhenPaused();
		
		Vector<StackFrame> stack;
		StackFrame& f1 = stack.Add();
		f1.file = "main.cpp";
		f1.line = 42;
		f1.function = "main";
		f1.address = "0x400500";
		
		if(WhenStack) WhenStack(stack);
		
		Vector<Variable> locals;
		Variable& v1 = locals.Add();
		v1.name = "argc";
		v1.value = "1";
		v1.type = "int";
		
		if(WhenLocals) WhenLocals(locals);
	});
}

void GdbService::Stop() {
	if(WhenOutput) WhenOutput("GDB Stopped.\n");
}

void GdbService::Step() {
	if(WhenOutput) WhenOutput("Step Into\n");
}

void GdbService::Next() {
	if(WhenOutput) WhenOutput("Step Over\n");
}

void GdbService::Cont() {
	if(WhenOutput) WhenOutput("Continuing...\n");
}

}