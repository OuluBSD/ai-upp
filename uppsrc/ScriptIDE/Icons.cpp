#include "ScriptIDE.h"
#include "Icons.h"

NAMESPACE_UPP

Image GetTablerIcon(const char *name, int size)
{
	static VectorMap<String, Image> cache;
	
	String theme = IsDarkTheme() ? "dark" : "light";
	String key;
	key << theme << ":" << name << ":" << size;
	
	int f = cache.Find(key);
	if(f >= 0) return cache[f];
	
	String path;
	// Try absolute path from workspace root first (for development)
	path = "/common/active/sblo/Dev/ai-upp/share/icons/";
	path << theme << "/tabler/outline/" << name << "_" << size << ".png";
	
	if(!FileExists(path)) {
		// Fallback to relative path from executable
		path = GetExeDirFile("../share/icons/");
		path << theme << "/tabler/outline/" << name << "_" << size << ".png";
	}
	
	Image img = StreamRaster::LoadFileAny(path);
	if(!img) {
		// Fallback placeholder if icon not found
		img = CtrlImg::help();
	}
	
	cache.Add(key, img);
	return img;
}

END_UPP_NAMESPACE
