#ifndef _MetaCtrl_Node_h_
#define _MetaCtrl_Node_h_

class WidgetCtrl : public Ctrl {
	
public:
	typedef WidgetCtrl CLASSNAME;
	WidgetCtrl();
	virtual ~WidgetCtrl();
	
	virtual void Data() = 0;
	virtual void DataTree(TreeCtrl& tree);
	virtual void ToolMenu(Bar& bar) = 0;
	virtual void EditPos(JsonIO& json) {};
	virtual String GetTitle() const {return String();}
	virtual Value* GetValue() {return 0;}
	virtual VfsPath GetCursorPath() const = 0;
	Event<> WhenTitle;
};

struct MetaExtCtrl : WidgetCtrl {
	Ptr<MetaNodeExt> ext;
	Event<> WhenEditorChange;
	Event<> WhenTitle;
	Ptr<ToolAppCtrl> owner;
	
	MetaNode& GetNode();
	const MetaNode& GetNode() const;
	MetaNodeExt& GetExt();
	String GetFilePath() const;
	VfsPath GetCursorPath() const override;
	
	template <class T> T& GetExt() {return dynamic_cast<T&>(*ext);}
};

#endif
