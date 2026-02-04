#ifndef _Maestro_InventoryGenerator_h_
#define _Maestro_InventoryGenerator_h_

struct MaestroInventoryFileInfo : Moveable<MaestroInventoryFileInfo> {
	String         path;
	String         full_path;
	int64          size = 0;
	String         hash;
	String         language;
	Vector<String> roles;

	void Jsonize(JsonIO& jio) {
		jio("path", path)("full_path", full_path)("size", size)("hash", hash)("language", language)("roles", roles);
	}
};

struct MaestroInventory : Moveable<MaestroInventory> {
	String                   repository_path;
	Array<MaestroInventoryFileInfo> files;
	int                      total_count = 0;
	VectorMap<String, int>   by_extension;
	VectorMap<String, int>   by_language;
	VectorMap<String, int>   by_role;
	VectorMap<String, int>   by_directory;
	ValueMap                 size_summary;
	ValueMap                 summary;

	void Jsonize(JsonIO& jio) {
		jio("repository_path", repository_path)
		   ("files", files)
		   ("total_count", total_count)
		   ("by_extension", by_extension)
		   ("by_language", by_language)
		   ("by_role", by_role)
		   ("by_directory", by_directory)
		   ("size_summary", size_summary)
		   ("summary", summary);
	}
};

class InventoryGenerator {
public:
	static MaestroInventory Generate(const String& repo_path);
	static String    DetectLanguage(const String& filename);
	static Vector<String> ClassifyFileRole(const String& filepath, const String& language);
	static String    GetFileHash(const String& filepath);
};

#endif
