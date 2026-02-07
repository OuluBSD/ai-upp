#include "WebDriver.h"
#include "Undetected.h"

NAMESPACE_UPP

namespace detail {

String GetFirefoxInstallationPath() {
#ifdef PLATFORM_POSIX
	static const char* paths[] = {
		"/usr/lib/firefox",
		"/usr/lib/firefox-esr",
		"/usr/lib/firefox-developer-edition",
		"/usr/lib/firefox-nightly",
	};
	for (const char* p : paths) {
		if (DirectoryExists(p)) return p;
	}
	// Fallback to searching via 'which'
	String out = Sys("which firefox");
	if (!out.IsEmpty()) {
		return GetFileFolder(TrimBoth(out));
	}
#endif
	return "";
}

String GetUndetectedFirefoxPath() {
	return GetHomeDirFile(AppendFileName(".cache", "undetected_firefox"));
}

String PatchFirefoxBinary() {
	String src = GetFirefoxInstallationPath();
	String dst = GetUndetectedFirefoxPath();
	
	if (src.IsEmpty()) return "";
	
	String xul_name = "libxul.so";
#ifdef PLATFORM_WIN32
	xul_name = "xul.dll";
#endif

	String dst_xul = AppendFileName(dst, xul_name);
	
	if (!DirectoryExists(dst)) {
		RealizeDirectory(dst);
		FindFile ff(AppendFileName(src, "*"));
		while (ff) {
			if (ff.IsFile()) {
				String s = LoadFile(ff.GetPath());
				SaveFile(AppendFileName(dst, ff.GetName()), s);
			}
			ff.Next();
		}
	}
	
	if (FileExists(dst_xul)) {
		String data = LoadFile(dst_xul);
		const char* target = "webdriver";
		int len = 9;
		
		String replacement;
		for (int i = 0; i < len; i++)
			replacement.Cat('a' + Random(26));
			
		int count = 0;
		int pos = data.Find(target);
		while (pos >= 0) {
			data.Remove(pos, len);
			data.Insert(pos, replacement);
			count++;
			pos = data.Find(target, pos + len);
		}
		
		if (count > 0) {
			SaveFile(dst_xul, data);
		}
	}
	
	String exe_name = "firefox";
#ifdef PLATFORM_WIN32
	exe_name = "firefox.exe";
#endif
	return AppendFileName(dst, exe_name);
}

} // namespace detail

END_UPP_NAMESPACE