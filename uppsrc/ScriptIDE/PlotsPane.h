#ifndef _ScriptIDE_PlotsPane_h_
#define _ScriptIDE_PlotsPane_h_

class PlotsPane : public DockableCtrl {
public:
	typedef PlotsPane CLASSNAME;
	PlotsPane();

	void AddPlot(const Image& img);
	void Clear();

	Event<> WhenSaveAll;
	Event<double> WhenZoom;

private:
	struct ImageDisplay : public Ctrl {
		Image img;
		double zoom = 1.0;
		virtual void Paint(Draw& w) override;
		void SetImage(const Image& _img) { img = _img; Refresh(); }
	};

	ToolBar toolbar;
	ImageDisplay display;
	Array<Image> plots;
	int current_index = -1;

	void UpdateDisplay();
	void SaveSelected();
	void SaveAll();
	void CopySelected();
	void RemoveSelected();
	void PrevPlot();
	void NextPlot();
	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);
};

#endif
