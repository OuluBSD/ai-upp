#ifndef _AI_AICodeCtrl_h_
#define _AI_AICodeCtrl_h_

struct AiAnnotationItem;
struct AiFileInfo;

NAMESPACE_UPP

struct AICodeCtrl : ParentCtrl {
	using SourceFile = AiAnnotationItem::SourceFile;
	Splitter			hsplit, rsplit;
	CodeEditor			editor;
	ArrayCtrl			cursorinfo, depthfirst;

	String				filepath;
	String				includes;
	String				aion_path;
	int					lineh = 24;
	Font				fnt;
	String				content;
	String				hash_sha1;
	int					sel_line = -1;
	SourceFile*			sel_ann_f = 0;
	AiAnnotationItem*	sel_ann = 0;
	AiFileInfo*			sel_f = 0;
	Color				clr_sel;
	Color				clr_ann;
	byte				charset = 0;
	Vector<int>			editor_to_line;
	Vector<int>			line_to_editor;
	Vector<int>			comment_to_line;
	CodeArgs			args;
	int					prev_editor_cursor = -1;
	
	typedef AICodeCtrl CLASSNAME;
	AICodeCtrl();
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
	void MakeAiComments();
	void CheckEditorCursor();
	void OnEditorCursor();
	void AnnotationData();
	static ArrayMap<String, AionFile>& AionFiles();
};

END_UPP_NAMESPACE

#endif
