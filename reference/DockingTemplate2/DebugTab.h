#ifndef _DockingTemplate2_DebugTab_h_
#define _DockingTemplate2_DebugTab_h_

class DebugLog : public ParentCtrl {
public:
	typedef DebugLog CLASSNAME;

	DebugLog();
	void Log(const String& msg);
	void Clear();

private:
	TextCtrl text_;
	Button   clear_btn_;
};

#endif
