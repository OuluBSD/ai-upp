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
	Vector<String> sections = Split(data, '[');
	
	String default_profile_path;
	String release_profile_path;
	String install_default_path;
	
	for (const String& section_content : sections) {
		Vector<String> lines = Split(section_content, '\n');
		if (lines.IsEmpty()) continue;
		
		String header = lines[0];
		bool is_profile = header.StartsWith("Profile");
		bool is_install = header.StartsWith("Install");
		
		if (!is_profile && !is_install) continue;

		String p_path;
		String p_name;
		bool is_relative = true;
		bool is_p_default = false;

		for (int i = 1; i < lines.GetCount(); i++) {
			String line = TrimBoth(lines[i]);
			if (line.StartsWith("Path=")) p_path = line.Mid(5);
			if (line.StartsWith("Name=")) p_name = line.Mid(5);
			if (line.StartsWith("Default=1")) is_p_default = true;
			if (line.StartsWith("IsRelative=0")) is_relative = false;
			if (is_install && line.StartsWith("Default=")) p_path = line.Mid(8);
		}
		
		if (p_path.IsEmpty()) continue;
		
		String full_path = is_relative ? AppendFileName(GetFileFolder(ini_path), p_path) : p_path;
		
		if (is_install) {
			install_default_path = full_path;
		} else if (is_profile) {
			// Prioritize "default-release" explicitly
			if (p_name == "default-release") release_profile_path = full_path;
			// Fallback to Default=1
			if (is_p_default) default_profile_path = full_path;
		}
	}
	
	if (!release_profile_path.IsEmpty()) {
		RLOG("WebDriver: Selected default-release profile: " << release_profile_path);
		return release_profile_path;
	}
	
	if (!install_default_path.IsEmpty()) {
		RLOG("WebDriver: Selected install-specific default profile: " << install_default_path);
		return install_default_path;
	}
	
	if (!default_profile_path.IsEmpty()) {
		RLOG("WebDriver: Selected default profile: " << default_profile_path);
		return default_profile_path;
	}
	
	return "";
}

} // namespace detail

END_UPP_NAMESPACE