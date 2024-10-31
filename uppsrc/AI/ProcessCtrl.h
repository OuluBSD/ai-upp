#ifndef _AI_ProcessCtrl_h_
#define _AI_ProcessCtrl_h_

NAMESPACE_UPP

// the priority value for one item is not just numerical, but preferably "structural": the
// class has a priority, which affects the priority of the class' Field and functions, etc.
struct AITaskPriority : Moveable<AITaskPriority>
{
	
};

struct AITask : Moveable<AITask>
{
	AITaskPriority priority;
	
};

struct AIProcess
{
	typedef enum : int {
		FN_BASE_ANALYSIS,
		FN_COUNT
	} FnType;
	Vector<AITask> tasks;
	FnType cur_fn;
	bool running = false, stopped = true;
	
	typedef AIProcess CLASSNAME;
	AIProcess();
	void Start(FnType fn);
	void Stop();
	void Run();
	void MakeBaseAnalysis();
	bool ProcessTask(AITask& t);
	
};

struct AIProcessCtrl : ParentCtrl
{
	AIProcess process;
	
	typedef AIProcessCtrl CLASSNAME;
	AIProcessCtrl();
	void RunTask(AIProcess::FnType fn);
};

END_UPP_NAMESPACE

#endif
