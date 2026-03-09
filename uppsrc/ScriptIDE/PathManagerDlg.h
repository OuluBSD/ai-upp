#ifndef _ScriptIDE_PathManagerDlg_h_
#define _ScriptIDE_PathManagerDlg_h_

class PathManagerDlg : public WithPYTHONPATHLayout<TopWindow> {
public:
	typedef PathManagerDlg CLASSNAME;
	PathManagerDlg();

	void Set(const PathManager& pm);
	void Get(PathManager& pm);

private:
	void OnAdd();
	void OnRemove();
};

#endif
