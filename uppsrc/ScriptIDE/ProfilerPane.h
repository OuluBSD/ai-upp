#ifndef _ScriptIDE_ProfilerPane_h_
#define _ScriptIDE_ProfilerPane_h_

class ProfilerPane : public DockableCtrl {
public:
	typedef ProfilerPane CLASSNAME;
	ProfilerPane();

	void SetData(const VectorMap<String, Value>& data); // Placeholder for actual profile data
	void Clear();

private:
	ArrayCtrl list;
};

#endif
