#ifndef _DockingTemplate2_DebugTab_h_
#define _DockingTemplate2_DebugTab_h_

class DebugLog : public ParentCtrl {
public:
	typedef DebugLog CLASSNAME;

	DebugLog();
	void AddRecord(const AppLogRecord& r);
	void Clear();

private:
	ArrayCtrl list_;
	Button    clear_btn_;

	static Color LevelColor(int level);
};

#endif
