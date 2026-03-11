#include "ScriptIDE.h"
#include <Painter/Painter.h>

NAMESPACE_UPP

static Image LoadSVG(const String& path, int size)
{
	String svg = LoadFile(path);
	if(svg.IsEmpty()) return Image();
	return RenderSVGImage(Size(size, size), svg);
}

static Image LoadPNG(const String& path)
{
	return StreamRaster::LoadFileAny(path);
}

Image GetIcon(const char *name, int size)
{
	static VectorMap<String, Image> cache;
	
	String theme = IsDarkTheme() ? "dark" : "light";
	String key;
	key << theme << ":" << name << ":" << size;
	
	int f = cache.Find(key);
	if(f >= 0) return cache[f];
	
	Image img;
	String base_dir = "/common/active/sblo/Dev/ai-upp/share/icons/";
	
	// 1. Try Spyder theme-specific SVG
	String path = base_dir;
	path << theme << "/spyder/" << name << ".svg";
	if(FileExists(path)) img = LoadSVG(path, size);
	
	// 2. Try Spyder common SVG
	if(!img) {
		path = base_dir << "spyder/" << name << ".svg";
		if(FileExists(path)) img = LoadSVG(path, size);
	}
	
	// 3. Try Tabler theme-specific PNG (Fallback)
	if(!img) {
		path = base_dir << theme << "/tabler/outline/" << name << "_" << size << ".png";
		if(FileExists(path)) img = LoadPNG(path);
	}
	
	// 4. Try Tabler common PNG (Fallback)
	if(!img) {
		path = base_dir << "tabler/outline/" << name << "_" << size << ".png";
		if(FileExists(path)) img = LoadPNG(path);
	}
	
	// 5. Hardcoded fallbacks for specific names if still missing
	if(!img) {
		if(String(name) == "run_selection") return GetIcon("player-play", size);
		if(String(name) == "project_new") return GetIcon("file-plus", size);
		if(String(name) == "project_open") return GetIcon("folder-open", size);
		if(String(name) == "run_settings") return GetIcon("device-floppy", size);
		if(String(name) == "ArchiveFileIcon") return GetIcon("file-stack", size);
		if(String(name) == "project_close") return GetIcon("player-stop", size);
		if(String(name) == "findprevious") return GetIcon("arrow-back-up", size);
		if(String(name) == "findnext") return GetIcon("arrow-forward-up", size);
		if(String(name) == "undo") return GetIcon("arrow-back-up", size);
		if(String(name) == "redo") return GetIcon("arrow-forward-up", size);
		if(String(name) == "filesave") return GetIcon("device-floppy", size);
		if(String(name) == "save_all") return GetIcon("file-stack", size);
	}

	if(!img) {
		// Final fallback placeholder
		img = CtrlImg::help();
	}
	
	cache.Add(key, img);
	return img;
}

END_UPP_NAMESPACE
