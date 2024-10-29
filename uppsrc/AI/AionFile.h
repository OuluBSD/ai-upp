#ifndef _AI_AionFile_h_
#define _AI_AionFile_h_

struct FileAnnotation;

NAMESPACE_UPP

bool MakeRelativePath(const String& includes, const String& dir, String& best_ai_dir, String& best_rel_dir);
Vector<String> FindParentUppDirectories(const String& dir);

struct AionFile {
	typedef AionFile CLASSNAME;
	ArrayMap<String, AiFileInfo> files;
	String saved_hash;
	
	
	AionFile() {}
	AionFile(AionFile&& f) {*this = f;}
	AionFile(const AionFile& f) {*this = f;}
	void operator=(const AionFile& f);
	void SetPath(String path);
	void Load();
	void Save(bool forced=false);
	void Clear();
	void PostSave();
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	AiFileInfo& RealizePath(const String& includes, const String& path);
	bool IsEmpty() const { return files.IsEmpty(); }
	String GetHashSha1();
	void Load(const String& includes, const String& path, FileAnnotation& fa);
	void Store(const String& includes, const String& path, FileAnnotation& fa);
private:
	
	bool post_saving = false;
	String path, dir;
	Mutex lock;
	
};

struct AionIndex {
	ArrayMap<String, AionFile> files;
	RWMutex lock;
	
	AionIndex() {}
	String ResolveAionFilePath(const String& includes, String path);
	AionFile& ResolveFile(const String& includes, String path);
	AiFileInfo& ResolveFileInfo(const String& includes, String path);
	void Load(const String& includes, const String& path, FileAnnotation& fa);
	void Store(const String& includes, const String& path, FileAnnotation& fa);
};

AionIndex& AiIndex();

END_UPP_NAMESPACE

#endif
