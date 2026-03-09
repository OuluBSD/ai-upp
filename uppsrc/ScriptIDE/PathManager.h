#ifndef _ScriptIDE_PathManager_h_
#define _ScriptIDE_PathManager_h_

class PathManager {
public:
	void AddPath(const String& path);
	void RemovePath(int index);
	void Clear() { paths.Clear(); }
	const Vector<String>& GetPaths() const { return paths; }

	void SyncToVM(PyVM& vm);
	
	void Serialize(Stream& s);

private:
	Vector<String> paths;
};

#endif
