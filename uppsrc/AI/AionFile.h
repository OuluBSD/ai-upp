#ifndef _AI_AionFile_h_
#define _AI_AionFile_h_

struct FileAnnotation;

NAMESPACE_UPP

struct AionFile {
	typedef AionFile CLASSNAME;
	ArrayMap<String, AiFileInfo> files;

	void SetPath(String path);
	void Load();
	void Save();
	void Clear();
	void PostSave();
	void Jsonize(JsonIO& json);
	AiFileInfo& RealizePath(const String& path);
	bool IsEmpty() const { return files.IsEmpty(); }

private:
	bool post_saving = false;
	String path, dir;
	RWMutex lock;
};

struct AionIndex {
	ArrayMap<String, AionFile> files;
	RWMutex lock;

	String ResolveAionFilePath(String path);
	AionFile& ResolveFile(String path);
	AiFileInfo& ResolveFileInfo(String path);
	void Load(const String& path, FileAnnotation& fa);
	void Store(const String& path, FileAnnotation& fa);
};

AionIndex& AiIndex();

END_UPP_NAMESPACE

#endif