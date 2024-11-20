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



#if 0

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
		if (task_i >= tasks.GetCount())
			break;
		
		if (waiting) {
			Sleep(10);
			continue;
		}
		
		AITask& t = tasks[task_i];
		if (!ProcessTask(t))
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
		
		if (it.have_ref) {
			if (!it.have_link) {
				LOG(i << ": " << it.ref.ToString() << ", error: NO LINK");
			}
			else {
				LOG(i << ": " << it.ref.ToString() << ", " << it.link.ToString());
			
				// TODO solve macro & add 'is_macro' to MakeTask
				
			}
		}
		else if (it.have_ann) {
			AITask& t = tasks.Add();
			t.filepath = filepath;
			t.vis = it;
			/*if (!it.have_link) {
				LOG(i << ": " << it.ann.ToString() << ", NO LINK");
			}
			else {
				LOG(i << ": " << it.ann.ToString() << ", " << it.link.ToString());
			}*/
			MakeTask(t);
		}
		
		i++;
	}
	
	SortTasks();
}

bool AITask::IsLinked(const AITask& t, const Relation& rel) const {
	bool is_any_type	= IsTypeKind(vis.ann.kind);
	bool is_any_var		= IsVarKind(vis.ann.kind);
	bool is_definition	= vis.ann.definition;
	bool is_any_fn		= IsFunctionAny(vis.ann.kind);
	bool is_macrodef	= vis.ann.kind == CXCursor_MacroDefinition;
	
	if (!vis.have_ann || !t.vis.have_ann)
		return false;
	
	switch (rel.reason) {
		case NO_REASON:
		break;
		
		case USAGE_REF: {
			if (is_any_var && rel.id == this->vis.ann.id)
				return true;
		}
		break;
		
		case USAGE_TYPE:
		case TYPE_INHERITANCE_DEPENDENCY:
		case TYPE_INHERITANCE_DEPENDING:
		case TYPE_USAGE: {
			if (is_any_type && rel.type == this->vis.ann.id)
				return true;
		}
		break;
		
		case USAGE_ADDR:
		case USAGE_CALL: {
			if (is_any_fn && rel.type == this->vis.ann.id)
				return true;
		}
		break;
		
		case USAGE_MACRO: {
			if (is_macrodef && rel.id == this->vis.ann.id)
				return true;
		}
		case METHOD: {
			if (is_any_type && rel.nest == this->vis.ann.id)
				return true;
		}
		break;
		
		case TYPE_PARENT: {
			if (is_any_type && rel.type == this->vis.ann.id)
				return true;
		}
		break;
		
		case RETURN_VALUE:
			break;
			
		#ifdef flagDEBUG
		default: Panic("TODO"); break;
		#endif
	}
	
	return false;
}

void AIProcess::FindDependencies(Array<SortItem>& sort_items, SortItem& s0) {
	for (const auto& rel : s0.task.relations) {
		if (!rel.is_dependency)
			continue;
		for (const SortItem& s1 : sort_items) {
			if (s1.task.IsLinked(s0.task, rel)) {
				s0.deps << &s1;
			}
		}
	}
}

void AIProcess::SortTasks() {
	Array<Vector<SortItem*>> tmp_items;
	Vector<SortItem*> remaining_items;
	Vector<SortItem*> sorted_items;
	Array<SortItem> sort_items;
	
	if (tasks.IsEmpty())
		return;
	
	{
		int i = 0;
		for (AITask& t : tasks) {
			sort_items.Add(new SortItem(t, i++));
		}
	}
	
	{
		Vector<SortItem*>& first_row = tmp_items.Add();
		for (SortItem& si : sort_items) {
			FindDependencies(sort_items, si);
			if (si.deps.IsEmpty())
				first_row.Add(&si);
			else
				remaining_items.Add(&si);
		}
	}
	
	if (tmp_items.Top().IsEmpty()) {
		AddError("empty first row: circular dependencies are possible");
		return;
	}
	
	while (!remaining_items.IsEmpty()) {
		Vector<SortItem*>& prev_row = tmp_items.Top();
		
		for (SortItem* si : prev_row)
			si->ready = true;
		
		Vector<SortItem*>& cur_row = tmp_items.Add();
		
		for(int i = 0; i < remaining_items.GetCount(); i++) {
			SortItem& si = *remaining_items[i];
			ASSERT(si.deps.GetCount());
			bool all_ready = true;
			for (auto* p : si.deps)
				if (!p->ready)
					all_ready = false;
			if (all_ready) {
				cur_row.Add(remaining_items[i]);
				remaining_items.Remove(i--);
			}
		}
		
		if (cur_row.IsEmpty()) {
			AddError("empty row: circular dependencies are possible");
			return;
		}
	}
	
	{
		int i = 0;
		for (auto& t : tasks)
			t.order = -1;
		for (const Vector<SortItem*>& vv : tmp_items) {
			for (const SortItem* v : vv) {
				v->task.order = i++;
			}
		}
		ASSERT(i == tasks.GetCount());
	}
	Sort(tasks, AITask());
	
	LOG("All done");
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

bool IsCallAny(int kind) {
	return	kind == CXCursor_CallExpr;
}

bool IsTypeKindBuiltIn(const String& s) {
	return	s == "int" ||
			s == "double" ||
			s == "char" ||
			s == "bool" ||
			s == "float" ||
			s == "long" ||
			s == "unsigned int" ||
			s == "short" ||
			s == "unsigned short" ||
			s == "long long" ||
			s == "unsigned long" ||
			s == "long double" ||
			s == "wchar_t";
}

bool AIProcess::MakeTask(AITask& t) {
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
	bool is_macrodef	= ann.kind == CXCursor_MacroDefinition;
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
				auto& rel = t.relations.Add();
				rel.reason = AITask::TYPE_INHERITANCE_DEPENDENCY;
				rel.type = s;
				rel.is_dependency = true;
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
								auto& rel = t.relations.Add();
								rel.reason = AITask::TYPE_INHERITANCE_DEPENDING,
								//rel.file = it.key;
								rel.file = it.file;
								rel.type = ai.id;
								rel.kind = ai.kind;
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
							auto& rel = t.relations.Add();
							rel.reason = AITask::TYPE_USAGE,
							//rel.file = it.key;
							rel.file = it.file;
							rel.id = ai.id;
							rel.type = ai.type;
							rel.kind = ai.kind;
						}
						break;
					}
				}
			}
			
			// Methods & Fields
			for(const auto& it : vis.export_items) {
				if (!it.have_ann)
					continue;
				const AnnotationItem& ai = it.ann;
				if (!RangeContains(ai.pos, begin, end))
					continue;
				bool is_fn = IsFunctionAny(ai.kind);
				bool is_var = IsVarKind(ai.kind);
				bool is_field = is_var && it.ann.nest == id;
				if (is_fn || is_field) {
					auto& rel = t.relations.Add();
					//in.is_dependency = true;
					if (is_fn)
						rel.reason = AITask::METHOD;
					else if (is_field)
						rel.reason = AITask::FIELD;
					rel.file = it.file;
					rel.id = it.ann.id;
					rel.nest = id;
					rel.type = it.ann.type;
					rel.file = it.file;
					rel.pos = it.ann.pos;
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
			if (ref.id == id && ref.ref_pos == ann.pos) {
				auto& rel = t.relations.Add();
				rel.reason = AITask::USAGE_REF;
				//rel.file = it.key;
				rel.id = id;
				rel.file = it.file;
				rel.pos = ref.pos;
				rel.ref_pos = ref.ref_pos;
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
						auto& rel = t.relations.Add();
						rel.reason = AITask::USAGE_TYPE;
						rel.is_dependency = true;
						rel.type = ai.id;
						//rel.file = it.key;
						rel.file = it.file;
						rel.pos = it.pos;
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
			auto& rel = t.relations.Add();
			rel.is_dependency = true;
			rel.reason = AITask::TYPE_PARENT;
			rel.type = ann.parent_type;
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
				if (!it.have_ref) continue;
				const ReferenceItem& ref = it.ref;
				if (ref.id == id && ref.ref_pos == ann.pos) {
					auto& rel = t.relations.Add();
					rel.reason = IsCallAny(ref.kind) ? AITask::USAGE_CALL : AITask::USAGE_ADDR;
					//rel.file = it.key;
					rel.id = id;
					rel.file = it.file;
					rel.pos = ref.pos;
					rel.ref_pos = ref.ref_pos;
				}
			}
			for(const auto& it : vis.export_items) {
				if (it.have_ann) {
					const AnnotationItem& ai = it.ann;
					if (!RangeContains(ai.pos, begin, end) || ai.id == id)
						continue;
					if (ai.type.IsEmpty() || IsTypeKindBuiltIn(it.ann.type))
						continue;
					auto& rel = t.relations.Add();
					rel.is_dependency = true;
					rel.reason = AITask::TYPE_USAGE;
					rel.file = it.file;
					rel.type = it.ann.type;
					rel.file = it.file;
					rel.pos = it.ann.pos;
				}
				/*if (it.have_ref) {
					const ReferenceItem& ref = it.ref;
					if (!RangeContains(ref.pos, begin, end) || ref.id == id)
						continue;
					
					LOG("REF: " + ref.ToString());
					
					
				}*/
			}
			
			// Return statements
			for(const auto& it : vis.export_items) {
				if (!it.have_stmt) continue;
				const ReferenceItem& ref = it.ref;
				if (!RangeContains(ref.pos, begin, end))
					continue;
				if (t.HasReason(AITask::RETURN_VALUE, ref.pos))
					continue;
				auto& rel = t.relations.Add();
				rel.reason = AITask::RETURN_VALUE;
				rel.kind = ref.kind;
				//rel.file = it.key;
				rel.file = it.file;
				rel.pos = ref.pos;
				rel.ref_pos = it.ann.end;
			}
			
			
			// TODO if method, check clang_getOverriddenCursors
			
			LOG(ann.ToString());
		}
	}
	
	//4. macros
	if (is_macrodef) {
		//	- these seem to be a bit difficult to identify, when theide's alt+u does not recognize their use
		//		- you have to stop and see what points to this
		//	- these are analyzed weakly or strongly if necessary (either with raw text or with clarified types)
		//	- as input are those that are marked for the macro
		//	- this explains the same things as the function or its parts: where to write, sync, async, conditionality
		//	- it may be that you need to ask the artificial intelligence if the macro is an inline function, inline field, inline switch, etc...
		//		- a bit like FOG's various "auto statements".
		for(const auto& it : vis.export_items) {
			if (!it.have_ref) continue;
			const ReferenceItem& ref = it.ref;
			if (ref.id == id && ref.ref_pos == ann.pos) {
				auto& rel = t.relations.Add();
				rel.reason = AITask::USAGE_MACRO;
				//rel.file = it.key;
				rel.id = id;
				rel.file = it.file;
				rel.pos = ref.pos;
				rel.ref_pos = ref.ref_pos;
			}
		}
	}
	
	
	return true;
}

void AIProcess::AddError(String msg) {
	auto& e = errors.Add();
	e.pos = Null;
	e.msg = msg;
}

void AIProcess::AddError(String filepath, Point pos, String msg) {
	auto& e = errors.Add();
	e.filepath = filepath;
	e.pos = pos;
	e.msg = msg;
}

bool AIProcess::ProcessTask(AITask& t) {
	ASSERT(!waiting);
	TaskMgr& m = AiTaskManager();
	
	if (t.filepath.IsEmpty() || !FileExists(t.filepath)) {
		AddError(t.filepath, Null, "filepath doesn't exist");
		return false;
	}
	String content = LoadFile(t.filepath);
	
	CodeArgs args;
	args.fn = CodeArgs::FUNCTIONALITY;
	args.code <<= GetStringArea(content, t.vis.ann.begin, t.vis.ann.end);
	for(const auto& rel : t.relations) {
		String k;
		k = AITask::GetReasonString(rel.reason) + ": ";
		if (rel.kind > 0)
			k += GetCursorKindName((CXCursorKind)rel.kind) + ": ";
		if (!rel.id.IsEmpty())
			k += rel.id + " ";
		if (!rel.nest.IsEmpty())
			k += " (" + rel.nest + ")";
		if (!rel.type.IsEmpty())
			k += ": " + rel.type;
		
		if (rel.reason == AITask::USAGE_REF ||
			rel.reason == AITask::RETURN_VALUE ||
			rel.reason == AITask::USAGE_MACRO) {
			Point begin = rel.pos;
			Point end = rel.pos;
			begin.x = 0;
			begin.y = max(0, begin.y-1);
			end.x = 0;
			end.y = end.y+2;
			String code = GetStringRange(content, begin, end);
			code.Replace("\r", "");
			code.Replace("\n", "\\n");
			k += "\"" + code + "\"";
		}
		
		k = TrimBoth(k);
		
		args.data.GetAdd(k);
	}
	
	waiting = true;
	m.GetCode(args, callback(this, &AIProcess::OnResult));
	return true;
}

void AIProcess::OnResult(String s) {
	
	LOG(s);
	
	task_i++;
	waiting = false;
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
	tasks.AddColumn("Order ");
	tasks.AddIndex("IDX");
	tasks.AddIndex("TYPE");
	tasks.ColumnWidths("2 2 1 1 1 1 1 1");
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
			tasks.Set(row, 7, t.order);
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
			tasks.Set(row, 7, t.order);
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
			tasks.Set(row, 7, t.order);
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
		const auto& rel = t.relations[i];
		info.Set(i, 0, rel.is_dependency ? "X" : "");
		info.Set(i, 1, AITask::GetReasonString(rel.reason));
		info.Set(i, 2, rel.file);
		info.Set(i, 3, rel.pos);
		info.Set(i, 4, rel.kind >= 0 ? GetCursorKindName((CXCursorKind)rel.kind) : String());
		info.Set(i, 5, rel.id);
		info.Set(i, 6, rel.type);
		info.Set(i, 7, rel.ref_pos);
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

bool AITask::HasReason(Reason r, Point pos) const {
	for (const auto& in : relations) {
		if (in.reason == r && in.pos == pos)
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


#endif

END_UPP_NAMESPACE
