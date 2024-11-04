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
	typedef enum : int {
		NO_REASON,
		USAGE_RW,
		USAGE_TYPE,
		TYPE_INHERITANCE_DEPENDENCY,
		TYPE_INHERITANCE_DEPENDING,
		TYPE_USAGE,
		TYPE_PARENT,
		
		REASON_COUNT
	} Reason;
	static String GetReasonString(int i) {
		switch (i){
			case NO_REASON:						return "No reason";
			case USAGE_RW:						return "Usage: R/W";
			case USAGE_TYPE:					return "Usage: Type";
			case TYPE_INHERITANCE_DEPENDENCY:	return "Inherited type";
			case TYPE_INHERITANCE_DEPENDING:	return "Inheriting this type";
			case TYPE_USAGE:					return "Type in use";
			case TYPE_PARENT:					return "Type as parent";
			default:							return "<error>";
		}
	}
	struct Relation : Moveable<Relation> {
		Reason reason = NO_REASON;
		bool is_dependency = false;
		String file;
		String id;
		String type;
		int kind = -1;
		Point pos = Null;
		Point ref_pos = Null;
	};
	
	AITaskPriority priority;
	CodeVisitor::Item vis;
	Vector<Relation> relations;
	String filepath;
	
	AITask() {}
	bool HasInput(const String& id, int kind) const;
	bool HasDepType(const String& id) const;
	int GetDependencyCount() const;
};

CodeVisitorProfile& BaseAnalysisProfile();

struct AIProcess
{
	typedef enum : int {
		FN_BASE_ANALYSIS,
		FN_COUNT
	} FnType;
	struct Error : Moveable<Error> {
		String filepath;
		Point pos;
		String msg;
	};
	Array<AITask> tasks;
	FnType cur_fn;
	bool running = false, stopped = true;
	FileAnnotation* item = 0;
	AiAnnotationItem::SourceRange* range = 0;
	Vector<String> code;
	Vector<Error> errors;
	String filepath;
	int file_idx = -1;
	CodeVisitor vis;
	
	typedef AIProcess CLASSNAME;
	AIProcess();
	void SetSource(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code);
	void Start(FnType fn);
	void Stop();
	void Run();
	void MakeBaseAnalysis();
	bool ProcessTask(AITask& t);
	void AddError(String filepath, Point pos, String msg);
	
};

struct AIProcessCtrl : ParentCtrl
{
	Splitter vsplit;
	ArrayCtrl tasks, info, errors;
	AIProcess process;
	
	typedef AIProcessCtrl CLASSNAME;
	AIProcessCtrl();
	void Data();
	void DataTask();
	void RunTask(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code, AIProcess::FnType fn);
};

END_UPP_NAMESPACE

#endif
