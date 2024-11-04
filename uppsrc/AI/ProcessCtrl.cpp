#include "AI.h"
#include <ide/clang/clang.h>
String GetCursorKindName(CXCursorKind cursorKind);


NAMESPACE_UPP


CodeVisitorProfile& BaseAnalysisProfile() {
	static CodeVisitorProfile p;
	using Prof = CodeVisitorProfile;
	ONCELOCK {
		p	.SetAllTrue() // Just set all true, why not?
		
		// Individual rules
			//.Set(Prof::VISIT_VARIABLE_TYPE_DECL, true)
			;
	};
	return p;
}




AIProcess::AIProcess() {
	
}

void AIProcess::SetSource(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code)
{
	this->filepath = filepath;
	this->item = &item;
	this->range = &range;
	this->code = pick(code);
	this->file_idx = CodeIndex().Find(filepath);
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
		
		
		break;
	}
	
	running = false;
	stopped = true;
}

void AIProcess::MakeBaseAnalysis() {
	vis.SetProfile(BaseAnalysisProfile());
	vis.SetNoLimit();
	vis.Begin();
	vis.Visit(filepath, *item, range->begin, range->end);
	
	tasks.Clear();
	tasks.Reserve(vis.export_items.GetCount());
	int i = 0;
	for(const auto& it : vis.export_items) {
		AITask& t = tasks.Add();
		t.filepath = filepath;
		t.vis = it;
		
		if (it.have_ref) {
			if (!it.have_link) {
				LOG(i << ": " << it.ref.ToString() << ", error: NO LINK");
			}
			else {
				LOG(i << ": " << it.ref.ToString() << ", " << it.link.ToString());
			
				// TODO solve macro & add 'is_macro' to ProcessTask
				
			}
		}
		else if (it.have_ann) {
			if (!it.have_link) {
				LOG(i << ": " << it.ann.ToString() << ", NO LINK");
			}
			else {
				LOG(i << ": " << it.ann.ToString() << ", " << it.link.ToString());
			}
		}
		ProcessTask(t);
		
		i++;
	}
}

bool IsTypeKind(int kind) {
	return	kind == CXCursor_StructDecl ||
			kind == CXCursor_ClassDecl ||
			kind == CXCursor_ClassTemplate ||
			kind == CXCursor_ClassTemplatePartialSpecialization
			;
}

bool IsVarKind(int kind) {
	return	kind == CXCursor_FieldDecl ||
			kind == CXCursor_VarDecl ||
			kind == CXCursor_ParmDecl;
}

bool IsFunctionAny(int kind) {
	return	kind == CXCursor_FunctionDecl ||
			kind == CXCursor_CXXMethod ||
			kind == CXCursor_Constructor ||
			kind == CXCursor_Destructor ||
			kind == CXCursor_FunctionTemplate;
}

bool IsTypeKindBuiltIn(const String& s) {
	return	s == "int" &&
			s == "double" &&
			s == "char" &&
			s == "bool" &&
			s == "float" &&
			s == "long" &&
			s == "unsigned int" &&
			s == "short" &&
			s == "unsigned short" &&
			s == "long long" &&
			s == "unsigned long" &&
			s == "long double" &&
			s == "wchar_t";
}

bool AIProcess::ProcessTask(AITask& t) {
	const AnnotationItem& ann = t.vis.ann;
	const String id = ann.id;
	const String type = ann.type;
	Point begin = ann.begin;
	Point end = ann.end;
	
	bool is_struct		= ann.kind == CXCursor_StructDecl;
	bool is_class		= ann.kind == CXCursor_ClassDecl;
	bool is_class_tmpl	= ann.kind == CXCursor_ClassTemplate;
	bool is_class_tmpls	= ann.kind == CXCursor_ClassTemplatePartialSpecialization;
	bool is_any_type	= IsTypeKind(ann.kind);
	bool is_var_field	= ann.kind == CXCursor_FieldDecl;
	bool is_var			= ann.kind == CXCursor_VarDecl;
	bool is_var_param	= ann.kind == CXCursor_ParmDecl;
	bool is_any_var		= IsVarKind(ann.kind);
	bool is_builtin		= is_any_var && IsTypeKindBuiltIn(ann.type);
	bool is_function	= ann.kind == CXCursor_FunctionDecl;
	bool is_method		= ann.kind == CXCursor_CXXMethod;
	bool is_constructor	= ann.kind == CXCursor_Constructor;
	bool is_destructor	= ann.kind == CXCursor_Destructor;
	bool is_definition	= ann.definition;
	bool is_any_fn		= IsFunctionAny(ann.kind);
	//const auto& idx = CodeIndex();
	//int t_file_idx = t.filepath.IsEmpty() ? -1 : idx.Find(t.filepath);
	
	/*
	TODO
	- CXCursor_ClassTemplatePartialSpecialization
	- typedefs
	- enums
	- etc. (see list)
	*/
	
	
	
	//1. Type: class, struct, template class, template class specialization, etc.
	if (is_any_type) {
		// is forward-declaration only
		if (!is_definition) {
			// useless?
		}
		// is definition
		else {
			//	- it needs the value of every Field and inherited class "function".
			Vector<String> bases = Split(ann.bases, ";");
			ASSERT(t.GetDependencyCount() == 0);
			for (String& s : bases) {
				auto& dep = t.relations.Add();
				dep.reason = AITask::TYPE_INHERITANCE_DEPENDENCY;
				dep.type = s;
				dep.is_dependency = true;
			}
			
			//	- it needs a list of classes that inherit from it (names/headers only)
			// loop all classes and check t.ann.bases;
			//for (auto it : ~idx) {
			//	for (const AnnotationItem& ai : it.value.items) {
			for(const auto& it : vis.export_items) {
				if (!it.have_ann) continue;
				const AnnotationItem& ai = it.ann;
				if  (ai.definition && IsTypeKind(ai.kind)) {
					Vector<String> bases0 = Split(ai.bases, ";");
					for (const auto& base0 : bases0) {
						if (base0 == ann.id) {
							if (!t.HasInput(ai.id, ai.kind)) {
								auto& in = t.relations.Add();
								in.reason = AITask::TYPE_INHERITANCE_DEPENDING,
								//in.file = it.key;
								in.file = it.file;
								in.id = ai.id;
								in.kind = ai.kind;
							}
							break;
						}
					}
				}
			}
			
			//	- it needs a list of classes and functions that use that class (names/headers only)
			// loop all vars/fields/params with the t.ann.type as this
			//for (auto it : ~idx) {
			//	for (const AnnotationItem& ai : it.value.items) {
			for(const auto& it : vis.export_items) {
				if (!it.have_ann) continue;
				const AnnotationItem& ai = it.ann;
				if (IsVarKind(ai.kind)) {
					if (ai.type == ann.id) {
						if (!t.HasInput(ai.id, ai.kind)) {
							auto& in = t.relations.Add();
							in.reason = AITask::TYPE_USAGE,
							//in.file = it.key;
							in.file = it.file;
							in.id = ai.id;
							in.kind = ai.kind;
						}
						break;
					}
				}
			}
			//LOG(t.ann.ToString());
		}
	}
	
	//2. Variable or class field (variable)
	if (is_any_var) {
		
		// TODO visit variable usage references in CodeVisitor
		
		
		//	- which functions use that field
		//		- there can easily be too much data here, so there is a limit to the number of lines
		//		- small functions entirely, and then the smallest suitable {} scope area, or crudely just a line-limited area
		//		- it is not necessary to examine the comments of the code, but only the raw codefor (auto it : ~idx) {
		//for (auto it : ~idx) {
		//	for (const ReferenceItem& ref : it.value.refs) {
		for(const auto& it : vis.export_items) {
			if (!it.have_ref) continue;
			const ReferenceItem& ref = it.ref;
			if (ref.id == id) {
				auto& in = t.relations.Add();
				in.reason = AITask::USAGE_RW;
				//in.file = it.key;
				in.id = id;
				in.file = it.file;
				in.pos = ref.pos;
				in.ref_pos = ref.ref_pos;
			}
		}
		
		// - type
		if (!is_builtin) {
			bool found = false;
			for(const auto& it : vis.export_items) {
				if (!it.have_ann) continue;
				const AnnotationItem& ai = it.ann;
				//for (auto it : ~idx) {
				//	for (const AnnotationItem& ai : it.value.items) {
				if (IsTypeKind(ai.kind) && ai.id == type) {
					if (!t.HasDepType(ai.id)) {
						auto& dep = t.relations.Add();
						dep.reason = AITask::USAGE_TYPE;
						dep.is_dependency = true;
						dep.type = ai.id;
						//dep.file = it.key;
						dep.file = it.file;
						dep.pos = it.pos;
						found = true;
					}
					break;
				}
				if (found) break;
				//}
				//if (found) break;
			}
		}
		//	- it is necessary to identify whether the variable has been used for synchronization: mutex, spinlock, etc...
		//		- artificial intelligence may recognize something else interesting
		// TODO improve this: this is very basic and dumb test
		String ltype = ToLower(ann.type);
		if (ltype.Find("mutex") >= 0 || ltype.Find("spinlock") >= 0) {
			LOG(ann.ToString());
			LOG("TODO: synchronization");
		}
	}
	
	//3. Methods and functions
	if (is_any_fn) {
		if (!ann.parent_type.IsEmpty()) {
			auto& dep = t.relations.Add();
			dep.is_dependency = true;
			dep.reason = AITask::TYPE_PARENT;
			dep.type = ann.parent_type;
		}
		
		// is forward-declaration only
		if (!is_definition) {
			// useless?
		}
		// is definition
		else {
			/*if (is_constructor ||
				is_destructor ||
				is_method ||
				is_conversion)*/
			/*if (t_file_idx < 0) {
				AddError(t.filepath, t.ann.pos, "no file idx found");
				return false;
			}*/
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
			//const auto& it = idx[t_file_idx];
			for(const auto& it : vis.export_items) {
				if (it.have_ann) {
					const AnnotationItem& ai = it.ann;
					if (!RangeContains(ai.pos, begin, end) || ai.id == id)
						continue;
					
					
					
					LOG("DEP: " + ai.ToString());
					
					// TODO ???
				}
				if (it.have_ref) {
					const ReferenceItem& ref = it.ref;
					if (!RangeContains(ref.pos, begin, end) || ref.id == id)
						continue;
					
					LOG("REF: " + ref.ToString());
					
					
				}
			}
			
			// TODO if method, check clang_getOverriddenCursors
			
			LOG(ann.ToString());
		}
	}
	
	//4. macros
	{
		//	- these seem to be a bit difficult to identify, when theide's alt+u does not recognize their use
		//		- you have to stop and see what points to this
		//	- these are analyzed weakly or strongly if necessary (either with raw text or with clarified types)
		//	- as input are those that are marked for the macro
		//	- this explains the same things as the function or its parts: where to write, sync, async, conditionality
		//	- it may be that you need to ask the artificial intelligence if the macro is an inline function, inline field, inline switch, etc...
		//		- a bit like FOG's various "auto statements".
	}
	
	
	return true;
}

void AIProcess::AddError(String filepath, Point pos, String msg) {
	auto& e = errors.Add();
	e.filepath = filepath;
	e.pos = pos;
	e.msg = msg;
}



AIProcessCtrl::AIProcessCtrl() {
	Add(vsplit.SizePos());
	
	vsplit.Vert() << tasks << info << errors;
	vsplit.SetPos(3333,0);
	vsplit.SetPos(6666+3333/2,1);
	
	/*
	String id; // Upp::Class::Method(Upp::Point p)
	String name; // Method
	String type; // for String x, Upp::String, surely valid for variables only
	String pretty; // void Class::Method(Point p)
	String nspace; // Upp
	String uname; // METHOD
	String nest; // Upp::Class
	String unest; // UPP::CLASS
	String bases; // base classes of struct/class
	*/
	tasks.AddColumn("Kind");
	tasks.AddColumn("Id");
	tasks.AddColumn("Type");
	tasks.AddColumn("Pretty");
	tasks.AddColumn("Bases");
	tasks.AddColumn("Pos");
	tasks.AddColumn("Parent Type");
	tasks.AddIndex("IDX");
	tasks.AddIndex("TYPE");
	tasks.ColumnWidths("2 2 1 1 1 1 1");
	tasks.WhenCursor << THISBACK(DataTask);
	
	info.AddColumn("Is dependency");
	info.AddColumn("Reason");
	info.AddColumn("File");
	info.AddColumn("Pos");
	info.AddColumn("Kind");
	info.AddColumn("Id");
	info.AddColumn("Type");
	info.AddColumn("Ref-pos");
	info.ColumnWidths("1 6 2 2 4 8 8 2");
	
	errors.AddColumn("File");
	errors.AddColumn("Pos");
	errors.AddColumn("Message");
	errors.ColumnWidths("4 1 8");
	
	
	
}

void AIProcessCtrl::Data() {
	int row = 0;
	for(int i = 0; i < process.tasks.GetCount(); i++) {
		const AITask& t = process.tasks[i];
		if (t.vis.have_ann) {
			const AnnotationItem& ann = t.vis.ann;
			tasks.Set(row, "IDX", i);
			tasks.Set(row, "TYPE", 0);
			tasks.Set(row, 0, ann.kind >= 0 ? GetCursorKindName((CXCursorKind)ann.kind) : String());
			tasks.Set(row, 1, ann.id);
			tasks.Set(row, 2, ann.type);
			tasks.Set(row, 3, ann.pretty);
			tasks.Set(row, 4, ann.bases);
			tasks.Set(row, 5, ann.pos);
			tasks.Set(row, 6, ann.parent_type);
			row++;
		}
		if (t.vis.have_ref) {
			const auto& ref = t.vis.ref;
			tasks.Set(row, "IDX", i);
			tasks.Set(row, "TYPE", 1);
			tasks.Set(row, 0, "REF");
			tasks.Set(row, 1, ref.id);
			tasks.Set(row, 2, Value());
			tasks.Set(row, 3, Value());
			tasks.Set(row, 4, Value());
			tasks.Set(row, 5, ref.pos.ToString() + " -> " + ref.ref_pos.ToString());
			tasks.Set(row, 6, Value());
			row++;
		}
		if (t.vis.have_link) {
			const AnnotationItem& ann = t.vis.link;
			tasks.Set(row, "IDX", i);
			tasks.Set(row, "TYPE", 2);
			tasks.Set(row, 0, ann.kind >= 0 ? GetCursorKindName((CXCursorKind)ann.kind) : String());
			tasks.Set(row, 1, ann.id);
			tasks.Set(row, 2, ann.type);
			tasks.Set(row, 3, ann.pretty);
			tasks.Set(row, 4, ann.bases);
			tasks.Set(row, 5, ann.pos);
			tasks.Set(row, 6, ann.parent_type);
			row++;
		}
	}
	tasks.SetCount(row);
	
	for(int i = 0; i < process.errors.GetCount(); i++) {
		const auto& e = process.errors[i];
		errors.Set(i, 0, e.filepath);
		errors.Set(i, 1, e.pos);
		errors.Set(i, 2, e.msg);
	}
	errors.SetCount(process.errors.GetCount());
	
	if (tasks.GetCount() && !tasks.IsCursor())
		tasks.SetCursor(0); // calls DataTask via WhenCursor callback
	else
		DataTask();
}

void AIProcessCtrl::DataTask() {
	int c = tasks.GetCursor();
	if (!tasks.IsCursor() || c < 0 || c >= process.tasks.GetCount()) {
		info.Clear();
		return;
	}
	int idx = tasks.Get("IDX");
	
	const AITask& t = process.tasks[idx];
	
	int row = 0;
	
	for(int i = 0; i < t.relations.GetCount(); i++) {
		const auto& in = t.relations[i];
		info.Set(i, 0, in.is_dependency ? "X" : "");
		info.Set(i, 1, AITask::GetReasonString(in.reason));
		info.Set(i, 2, in.file);
		info.Set(i, 3, in.pos);
		info.Set(i, 4, in.kind >= 0 ? GetCursorKindName((CXCursorKind)in.kind) : String());
		info.Set(i, 5, in.id);
		info.Set(i, 6, in.type);
		info.Set(i, 7, in.ref_pos);
	}
	info.SetCount(t.relations.GetCount());
	
	
}

void AIProcessCtrl::RunTask(String filepath, FileAnnotation& item, AiAnnotationItem::SourceRange& range, Vector<String> code, AIProcess::FnType fn) {
	process.SetSource(filepath, item, range, pick(code));
	process.Start(fn);
}




bool AITask::HasInput(const String& id, int kind) const {
	for (const auto& in : relations) {
		if (in.id == id && in.kind == kind)
			return true;
	}
	return false;
}

bool AITask::HasDepType(const String& id) const {
	for (const auto& in : relations) {
		if (in.is_dependency && !in.type.IsEmpty() && in.type == id)
			return true;
	}
	return false;
}

int AITask::GetDependencyCount() const {
	int i = 0;
	for (const auto& in : relations)
		if (in.is_dependency)
			i++;
	return i;
}


END_UPP_NAMESPACE
