#ifndef _ScriptIDE_PlotsPane_h_
#define _ScriptIDE_PlotsPane_h_

class PlotsPane : public ParentCtrl {
public:
	typedef PlotsPane CLASSNAME;
	PlotsPane();

	void AddPlot(const Image& img);
	void Clear();

private:
	struct ImageDisplay : public Ctrl {
		Image img;
		virtual void Paint(Draw& w) override;
		void SetImage(const Image& _img) { img = _img; Refresh(); }
	};

	ToolBar toolbar;
	ImageDisplay display;
	Array<Image> plots;
	int current_index = -1;

	void UpdateDisplay();
	void SaveSelected();
	void CopySelected();
	void PrevPlot();
	void NextPlot();
	void LayoutToolbar(Bar& bar);
};

#endif
