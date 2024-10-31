#include "AI.h"

NAMESPACE_UPP


AIProcess::AIProcess() {
	
}

void AIProcess::Start(FnType fn) {
	Stop();
	running = true;
	stopped = false;
	if ((int)fn < 0 || fn >= FN_COUNT) return;
	cur_fn = fn;
	Thread::Start(THISBACK(Run));
}

void AIProcess::Stop() {
	running = false;
	while (!stopped) Sleep(100);
}

void AIProcess::Run() {
	switch (cur_fn) {
	case AIProcess::FN_BASE_ANALYSIS:
		MakeBaseAnalysis();
		break;
	default:
		break;
	}
	
	while (!Thread::IsShutdownThreads() && running) {
		
		// process tasks
		
		Sleep(10);
	}
	
	running = false;
	stopped = true;
}

void AIProcess::MakeBaseAnalysis() {
	CodeVisitor vis;
	vis.SetLimit(1000);
	vis.Begin();
	//vis.Visit(filepath, fa, ann_f.begin, ann_f.end);
}

bool AIProcess::ProcessTask(AITask& t) {
	
	//1. Fruit category
	//	- it needs the value of every Field and inherited class "function".
	//	- it needs a list of classes that inherit from it (names/headers only)
	//	- it needs a list of classes and functions that use that class (names/headers only)
	
	//2. Fruit class fields
	//	- which functions use that field
	//		- there can easily be too much data here, so there is a limit to the number of lines
	//		- small functions entirely, and then the smallest suitable {} scope area, or crudely just a line-limited area
	//		- it is not necessary to examine the comments of the code, but only the raw code
	//	- it is necessary to identify whether the variable has been used for synchronization: mutex, spinlock, etc...
	//		- artificial intelligence may recognize something else interesting
	
	//3. Fruit class methods
	//	- comments/analysis of function content
	//		- list of object/class variables used in the function
	//			- the fields are preferably cleared before this, but the priority is lower
	//		- list of internal function variables
	//			- these types will be sorted out first with pleasure, but the priority is lower
	//		- list of global variables that are used
	//			- clearing these first has the "highest of WEAK priorities"
	//		- list of macros that are used
	//			- with these high of low priorities
	//		- a list of inline classes within the function
	//			- these will definitely be clarified at some level first
	//		- list of function parameters
	//			- we'd like to figure out the types first, but this is a lighter priority perhaps
	//		- list of function qualifiers: const, static, etc.
	//		- list of override functions to be overridden
	//			- these will be clarified first
	//		- list of override functions that can override this (classes that inherit this class and have that function)
	//			- of these, only raw data is given: ID, etc
	//		- a list of functions that are called in the function
	//			- these may be forward declare, in which case they may be unknown
	//				- the function can be required to perform a "weak analysis" in advance, which gives a superficial analysis of the raw data alone
	//	- large functions can be required to be split into several parts
	//		- this is a real requirement, which can be expected to be met for sure... because the input is quite small
	//		- the changes made by previous parts need to be received as input (what new information is in the variables)
	//	- information about the function or its parts is required
	//		- what changes the function writes and where
	//		- what synchronizations the function does with mutexes
	//		- based on which variable the function works conditionally (preferably in order of importance or with an importance value)
	//		- what asynchronous calls the function makes (callbacks, remote commands, function pointers)
	
	//4. macros
	//	- these seem to be a bit difficult to identify, when theide's alt+u does not recognize their use
	//		- you have to stop and see what points to this
	//	- these are analyzed weakly or strongly if necessary (either with raw text or with clarified types)
	//	- as input are those that are marked for the macro
	//	- this explains the same things as the function or its parts: where to write, sync, async, conditionality
	//	- it may be that you need to ask the artificial intelligence if the macro is an inline function, inline field, inline switch, etc...
	//		- a bit like FOG's various "auto statements".
	
	
	
	return true;
}



AIProcessCtrl::AIProcessCtrl() {
	
}

void AIProcessCtrl::RunTask(AIProcess::FnType fn) {
	process.Start(fn);
}

END_UPP_NAMESPACE
