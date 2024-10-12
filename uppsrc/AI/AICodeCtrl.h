#ifndef _AI_AICodeCtrl_h_
#define _AI_AICodeCtrl_h_


NAMESPACE_UPP


struct AICodeCtrl : Ctrl {
	Ide* ide = 0;
	String filepath;
	String aion_path;
	Vector<String> srclines;
	ScrollBar vscroll;
	int lineh = 24;
	Font fnt;
	String content;
	int sel_line = -1;
	Color clr_sel;
	
	AICodeCtrl();
	void SetIde(Ide* ide);
	void SetFont(Font fnt);
	void Load(String filename, Stream& str, byte charset);
	void Save(Stream& str, byte charset);
	void SetEditPos(LineEdit::EditPos pos);
	void SetPickUndoData(LineEdit::UndoData pos);
	LineEdit::UndoData PickUndoData();
	LineEdit::EditPos GetEditPos();
	void Paint(Draw& draw) override;
	void Layout() override;
	void MouseWheel(Point p, int zdelta, dword keyflags) override;
	void LeftDown(Point p, dword flags) override;
	static ArrayMap<String,AionFile>& AionFiles();
};


END_UPP_NAMESPACE


#endif
