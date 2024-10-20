#ifndef _AI_AionFile_h_
#define _AI_AionFile_h_


struct FileAnnotation;


NAMESPACE_UPP


struct AionFile
{
	typedef AionFile CLASSNAME;
	bool post_saving = false;
	String path;
	ArrayMap<String, String> source_files;
	RWMutex lock;
	
	void Load(String path);
	void Save();
	void Clear();
	void PostSave();
	void Jsonize(JsonIO& json);
	
	void Store(String path, FileAnnotation& f);
	void Load(String path, FileAnnotation& f);
	
private:
	String& RealizePathString(const String& path);
	
};

struct AionIndex {
	ArrayMap<String, AionFile> files;
	RWMutex lock;
	
	String ResolveAionFilePath(String path);
	AionFile& ResolveFile(String path);
	
};

AionIndex& AiIndex();

END_UPP_NAMESPACE


#endif
