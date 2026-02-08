#include "WebDriver.h"
#include "Undetected.h"

NAMESPACE_UPP

namespace detail {

String GetFirefoxInstallationPath() {
#ifdef PLATFORM_POSIX
	static const char* paths[] = {
		"/usr/lib/firefox",
		"/usr/lib64/firefox",
		"/usr/lib/firefox-esr",
		"/usr/lib/firefox-developer-edition",
		"/usr/lib/firefox-nightly",
		"/opt/firefox",
		"/usr/local/lib/firefox",
		"/usr/local/lib64/firefox",
		"/opt/firefox-bin",
	};
	for (const char* p : paths) {
		RLOG("WebDriver: Checking path: " << p);
		if (DirectoryExists(p) && FileExists(AppendFileName(p, "libxul.so"))) {
			RLOG("WebDriver: Found Firefox at: " << p);
			return p;
		}
	}
	
	// Try to find via binary path
	String out = TrimBoth(Sys("which firefox"));
	RLOG("WebDriver: 'which firefox' returned: " << out);
	if (!out.IsEmpty()) {
		String real_path = out;
		// Follow symlinks to find real installation
		for (int i = 0; i < 10; i++) {
			String link = GetSymLinkPath(real_path);
			if (link.IsEmpty()) break;
			RLOG("WebDriver: Following symlink: " << real_path << " -> " << link);
			real_path = NormalizePath(link, GetFileFolder(real_path));
		}
		String dir = GetFileFolder(real_path);
		RLOG("WebDriver: Checking dir of binary: " << dir);
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
		
		// Some distros have 'firefox' as a script in /usr/bin, but libs in /usr/lib/firefox
		dir = "/usr/lib/firefox";
		if (FileExists(AppendFileName(dir, "libxul.so"))) return dir;
	}
#endif
	RLOG("WebDriver: Firefox installation path NOT FOUND");
	return "";
}

String GetUndetectedFirefoxPath() {
	String home_bin = GetHomeDirFile("bin");
	String path_env = GetEnv("PATH");
	if (DirectoryExists(home_bin) && path_env.Find(home_bin) >= 0) {
		return AppendFileName(home_bin, "undetected_firefox");
	}
	return GetHomeDirFile(AppendFileName(".cache", "undetected_firefox"));
}

String PatchFirefoxBinary() {
	String src = GetFirefoxInstallationPath();
	String dst = GetUndetectedFirefoxPath();
	
	RLOG("WebDriver: PatchFirefoxBinary src=" << src << " dst=" << dst);
	
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
		src_xul = AppendFileName(AppendFileName(src, "browser"), xul_name);
		if (!FileExists(src_xul)) {
			RLOG("WebDriver: Warning: Could not find " + xul_name + " in " + src);
			return "";
		}
	}

	String exe_name = "firefox";
#ifdef PLATFORM_WIN32
	exe_name = "firefox.exe";
#endif
	String dst_exe = AppendFileName(dst, exe_name);

	if (!FileExists(dst_exe)) {
		RLOG("WebDriver: Creating undetected Firefox copy in " + dst);
		RealizeDirectory(dst);
		
		// Use shell to copy everything recursively
		String cmd = "cp -r " + src + "/* " + dst + "/";
		RLOG("WebDriver: Executing " << cmd);
		system(~cmd);
		
		// If only firefox-bin exists, copy it as firefox
		if (!FileExists(AppendFileName(dst, "firefox")) && FileExists(AppendFileName(dst, "firefox-bin"))) {
			RLOG("WebDriver: Copying firefox-bin to firefox");
			SaveFile(AppendFileName(dst, "firefox"), LoadFile(AppendFileName(dst, "firefox-bin")));
		}
		
		// Mark executable
#ifdef PLATFORM_POSIX
		chmod(AppendFileName(dst, "firefox"), 0755);
		if (FileExists(AppendFileName(dst, "firefox-bin")))
			chmod(AppendFileName(dst, "firefox-bin"), 0755);
		
		// Fix RPATH to ensure patched libxul.so is loaded from dst, not src
		String patchelf_cmd = "patchelf --set-rpath '$ORIGIN' " + dst_exe;
		RLOG("WebDriver: Executing " << patchelf_cmd);
		system(~patchelf_cmd);
#endif
	} else {
		RLOG("WebDriver: Undetected Firefox already exists at " << dst_exe);
	}
	
	// Re-verify dst_xul location in copy
	if (!FileExists(dst_xul)) {
		dst_xul = AppendFileName(AppendFileName(dst, "browser"), xul_name);
	}
	
	if (FileExists(dst_xul)) {
		String data = LoadFile(dst_xul);
		const char* target = "webdriver";
		int len = 9;
		
		int pos = data.Find(target);
		if (pos >= 0) {
			RLOG("WebDriver: Patching " + dst_xul);
			
			String replacement;
			for (int i = 0; i < len; i++)
				replacement.Cat('a' + Random(26));
			
			RLOG("WebDriver: Patching " << target << " with " << replacement << " in " << dst_xul);
				
			while (pos >= 0) {
				for (int i = 0; i < len; i++)
					data.Set(pos + i, replacement[i]);
				
				pos = data.Find(target, pos + len);
			}
			SaveFile(dst_xul, data);
		} else {
			RLOG("WebDriver: " << dst_xul << " already patched or does not contain '" << target << "'");
		}
	}
	
	RLOG("WebDriver: Returning patched binary path: " << dst_exe);
	return dst_exe;
}

String GetFirefoxDefaultProfilePath() {
	String ini_path;
#ifdef PLATFORM_POSIX
	ini_path = GetHomeDirFile(".mozilla/firefox/profiles.ini");
#elif defined(PLATFORM_WIN32)
	ini_path = GetAppDataDir() + "/Mozilla/Firefox/profiles.ini";
#endif

	if (!FileExists(ini_path)) return "";

	String data = LoadFile(ini_path);
	String base_dir = GetFileFolder(ini_path);
	
	String newest_profile_path;
	Time newest_time = Time::Low();
	
	// Better INI parsing
	Vector<String> lines = Split(data, '\n');
	String current_path;
	
	for (int i = 0; i < lines.GetCount(); i++) {
		String line = TrimBoth(lines[i]);
		if (line.StartsWith("Path=")) {
			String rel_path = line.Mid(5);
			String full_path = AppendFileName(base_dir, rel_path);
			
			if (DirectoryExists(full_path)) {
				// Check times of multiple files to find the most recently used profile
				static const char* activity_files[] = { "prefs.js", "places.sqlite", "sessionstore.jsonlz4", "lock" };
				Time max_t = Time::Low();
				
				for (const char* f : activity_files) {
					Time t = FileGetTime(AppendFileName(full_path, f));
					if (!IsNull(t) && t > max_t) max_t = t;
				}
				
				if (IsNull(max_t)) max_t = FileGetTime(full_path);
				
				if (!IsNull(max_t) && max_t > newest_time) {
					newest_time = max_t;
					newest_profile_path = full_path;
				}
			}
		}
	}
	
	if (!newest_profile_path.IsEmpty()) {
		RLOG("WebDriver: Selected most recent profile: " << newest_profile_path << " (" << newest_time << ")");
		return newest_profile_path;
	}
	
	return "";
}

} // namespace detail

END_UPP_NAMESPACE