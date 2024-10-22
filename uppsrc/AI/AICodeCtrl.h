#ifndef _AI_AICodeCtrl_h_
#define _AI_AICodeCtrl_h_

struct AiAnnotationItem;
struct AiFileInfo;

NAMESPACE_UPP

struct AICodeCtrl : ParentCtrl {
	SplitterFrame navigator;
	Splitter split;
	CodeEditor editor;
	ArrayCtrl itemlist, commentlist, datalist;

	String filepath;
	String aion_path;
	int lineh = 24;
	Font fnt;
	String content;
	int sel_line = -1;
	AiAnnotationItem* sel_ann = 0;
	AiFileInfo* sel_f = 0;
	Color clr_sel, clr_ann;
	byte charset = 0;
	Vector<int> editor_to_line, line_to_editor, comment_to_line;

	typedef AICodeCtrl CLASSNAME;
	AICodeCtrl();
	void SetFont(Font fnt);
	void Load(String filename, Stream& str, byte charset);
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
	void MakeAiComments();
	static ArrayMap<String, AionFile>& AionFiles();
};

END_UPP_NAMESPACE

#endif
