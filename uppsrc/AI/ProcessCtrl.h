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
	FileAnnotation* item = 0;
	AiAnnotationItem::SourceRange* range = 0;
	Vector<String> code;
	String filepath;
	
	typedef AIProcess CLASSNAME;
	AIProcess();
	void SetSource(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code);
	void Start(FnType fn);
	void Stop();
	void Run();
	void MakeBaseAnalysis();
	bool ProcessTask(AITask& t, const AnnotationItem* ann, const ReferenceItem* ref);
	
};

struct AIProcessCtrl : ParentCtrl
{
	AIProcess process;
	
	typedef AIProcessCtrl CLASSNAME;
	AIProcessCtrl();
	void RunTask(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code, AIProcess::FnType fn);
};

END_UPP_NAMESPACE

#endif
