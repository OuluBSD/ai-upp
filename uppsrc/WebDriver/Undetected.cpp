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
		if (DirectoryExists(p) && FileExists(AppendFileName(p, "libxul.so"))) return p;
	}
	
	// Try to find via binary path
	String out = TrimBoth(Sys("which firefox"));
	if (!out.IsEmpty()) {
		String real_path = out;
		// Follow symlinks to find real installation
		for (int i = 0; i < 10; i++) {
			String link = GetSymLinkPath(real_path);
			if (link.IsEmpty()) break;
			real_path = NormalizePath(link, GetFileFolder(real_path));
		}
		String dir = GetFileFolder(real_path);
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
		
		// Some distros have 'firefox' as a script in /usr/bin, but libs in /usr/lib/firefox
		dir = "/usr/lib/firefox";
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
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
	
	if (src.IsEmpty()) {
		RLOG("WebDriver: Warning: Could not find Firefox installation path. Binary patching skipped.");
		return "";
	}
	
	String xul_name = "libxul.so";
#ifdef PLATFORM_WIN32
	xul_name = "xul.dll";
#endif

	String src_xul = AppendFileName(src, xul_name);
	String dst_xul = AppendFileName(dst, xul_name);
	
	if (!FileExists(src_xul)) {
		RLOG("WebDriver: Warning: Could not find " + xul_name + " in " + src);
		return "";
	}

	String exe_name = "firefox";
#ifdef PLATFORM_WIN32
	exe_name = "firefox.exe";
#endif
	String dst_exe = AppendFileName(dst, exe_name);

	if (!FileExists(dst_exe)) {
		RLOG("WebDriver: Creating undetected Firefox copy in " + dst);
		RealizeDirectory(dst);
		
		// Copy essential files only to avoid massive copies
		static const char* essentials[] = { "firefox", "firefox-bin", "libxul.so", "omni.ja", "platform.ini", "dependentlibs.list" };
		for (const char* e : essentials) {
			String s_path = AppendFileName(src, e);
			if (FileExists(s_path)) {
				SaveFile(AppendFileName(dst, e), LoadFile(s_path));
			}
		}
		
		// Mark executable
#ifdef PLATFORM_POSIX
		chmod(AppendFileName(dst, "firefox"), 0755);
		if (FileExists(AppendFileName(dst, "firefox-bin")))
			chmod(AppendFileName(dst, "firefox-bin"), 0755);
#endif
	}
	
	if (FileExists(dst_xul)) {
		String data = LoadFile(dst_xul);
		const char* target = "webdriver";
		int len = 9;
		
		int pos = data.Find(target);
		if (pos >= 0) {
			RLOG("WebDriver: Patching " + xul_name);
			
			String replacement;
			for (int i = 0; i < len; i++)
				replacement.Cat('a' + Random(26));
				
			while (pos >= 0) {
				for (int i = 0; i < len; i++)
					data.Set(pos + i, replacement[i]);
				
				pos = data.Find(target, pos + len);
			}
			SaveFile(dst_xul, data);
		}
	}
	
	return dst_exe;
}

} // namespace detail

END_UPP_NAMESPACE