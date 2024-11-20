#ifndef _AI_AICodeCtrl_h_
#define _AI_AICodeCtrl_h_

struct AiAnnotationItem;
struct MetaSrcFile;

NAMESPACE_UPP

struct AICodeCtrl : ParentCtrl {
	void SetFont(Font fnt) {}
	void SetEditPos(LineEdit::EditPos pos) {}
	void SetPickUndoData(LineEdit::UndoData pos) {}
	void Load(const String& includes, String filename, Stream& str, byte charset) {}
	LineEdit::UndoData PickUndoData() {return LineEdit::UndoData();}
	LineEdit::EditPos GetEditPos() {return LineEdit::EditPos();}
	void Save(Stream& str, byte charset) {}
};

#if 0
struct AICodeCtrl : ParentCtrl {
	using SourceRange = AiAnnotationItem::SourceRange;
	Splitter			hsplit, rsplit;
	TabCtrl				tabs;
	CodeEditor			editor;
	ArrayCtrl			cursorinfo, depthfirst;
	AIProcessCtrl		process;

	String				filepath;
	String				includes;
	String				aion_path;
	int					lineh = 24;
	Font				fnt;
	String				content;
	int					sel_line = -1;
	SourceRange*		sel_ann_f = 0;
	AiAnnotationItem*	sel_ann = 0;
	MetaSrcFile*			sel_f = 0;
	Color				clr_sel;
	Color				clr_ann;
	byte				charset = 0;
	Vector<int>			editor_to_line;
	Vector<int>			line_to_editor;
	Vector<int>			comment_to_line;
	CodeArgs			args;
	int					prev_editor_cursor = -1;
	TimeCallback		tc;
	
	typedef AICodeCtrl CLASSNAME;
	AICodeCtrl();
	void OnTab();
	void SetFont(Font fnt);
	void Load(const String& includes, String filename, Stream& str, byte charset);
	void Save(Stream& str, byte charset);
	void SetEditPos(LineEdit::EditPos pos);
	void SetPickUndoData(LineEdit::UndoData pos);
	LineEdit::UndoData PickUndoData();
	LineEdit::EditPos GetEditPos();
	void StoreAion();
	void SetSelectedLineFromEditor();
	void SetSelectedAnnotationFromLine();
	void ContextMenu(Bar& bar);
	void AddComment();
	void RemoveComment();
	void UpdateEditor();
	Vector<String> GetAnnotationAreaCode();
	void MakeAiComments();
	void RunTask(AIProcess::FnType t);
	void CheckEditorCursor();
	void OnEditorCursor();
	void AnnotationData();
	static ArrayMap<String, MetaSrcPkg>& MetaSrcPkgs();
};
#endif

END_UPP_NAMESPACE

#endif
