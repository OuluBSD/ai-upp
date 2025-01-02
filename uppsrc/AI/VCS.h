#ifndef _AI_VCS_h_
#define _AI_VCS_h_

NAMESPACE_UPP


class VersionControlSystem {
	bool storing = false;
	String dir;
	
	typedef enum {
		ST_OBJECT,
		ST_VECTOR,
		ST_MAP,
		ST_MAP_KV,
	} ScopeType;
	struct Scope : Moveable<Scope> {
		String name;
		ScopeType type;
	};
	Vector<Scope> scopes;
	
	Scope& Push(ScopeType t, String name);
	void Pop();
	void RemoveIdxFolders(int begin);
public:
	typedef VersionControlSystem CLASSNAME;
	VersionControlSystem();
	~VersionControlSystem();
	void Initialize(String path);
	void Close();
	bool IsStoring() const;
	void SetStoring();
	void SetLoading();
	void BeginObject(String key);
	void BeginVector(String key, int count);
	void BeginMap(String key, int count);
	void BeginAt(int i);
	void BeginKey(int i, String key);
	void End();
	
	
	
	template <class T>
	void BeginVector(String key, T& o) {
		Push(ST_VECTOR, key);
		if (storing)
			RemoveIdxFolders(o.GetCount());
		else {
			int count = ResolveCount();
			o.SetCount(count);
		}
	}
	
	template <class T>
	void BeginMap(String key, T& o) {
		Push(ST_MAP, key);
		if (storing)
			RemoveIdxFolders(o.GetCount());
		else {
			Panic("TODO");
		}
	}

	template <class T>
	void Do(String key, T& o) {
		int a = key.Find(")");
		if (a >= 0) key = key.Mid(a+1);
		if (storing) {
			StoreAsJsonFile(o, GetCurrentPath(key + ".json"), true);
		}
		else {
			LoadFromJsonFile(o, GetCurrentPath(key + ".json"));
		}
	}
	String GetCurrentDirectory() const;
	String GetCurrentPath(String name) const;
	int ResolveCount() const;
};

END_UPP_NAMESPACE

#endif
