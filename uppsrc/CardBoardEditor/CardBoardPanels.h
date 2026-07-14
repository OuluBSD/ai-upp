#ifndef _CardBoardEditor_CardBoardPanels_h_
#define _CardBoardEditor_CardBoardPanels_h_

class CardBoardTreePanel : public DockableCtrl {
public:
	typedef CardBoardTreePanel CLASSNAME;
	CardBoardTreePanel();
	void SetDocument(const CardBoardDocument& document);

private:
	TreeCtrl tree_;
	void AddElement(int parent, const CardBoardElement& element);
};

class CardBoardPropertiesPanel : public DockableCtrl {
public:
	typedef CardBoardPropertiesPanel CLASSNAME;
	CardBoardPropertiesPanel();
	void SetDocument(const CardBoardDocument& document);

private:
	ArrayCtrl properties_;
};

class CardBoardDiagnosticsPanel : public DockableCtrl {
public:
	typedef CardBoardDiagnosticsPanel CLASSNAME;
	CardBoardDiagnosticsPanel();
	void SetText(const String& text);

private:
	LineEdit text_;
};

#endif
