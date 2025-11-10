#include "Core.h"

int GetRepo(String& path)
{
	if(IsNull(path))
		return NOT_REPO_DIR;
	if(DirectoryExists(AppendFileName(path, ".svn")) || DirectoryExists(AppendFileName(path, "_svn")))
		return SVN_DIR;
	for(;;) {
		String git = AppendFileName(path, ".git");
		if(DirectoryExists(git) || FileExists(git))
			return GIT_DIR;
		if(DirectoryExists(AppendFileName(path, ".svn")))
			return SVN_DIR;
		String path0 = path;
		path = GetFileFolder(path);
		if(path == path0)
			break;
	}
	return NOT_REPO_DIR;
}

int GetRepoKind(const String& p)
{
	String pp = p;
	return GetRepo(pp);
}

String GetGitPath()
{
#ifdef PLATFORM_WIN32
	static String path;
	ONCELOCK {
		path = AppendFileName(GetExeFolder(), "\\bin\\mingit\\cmd\\git.exe");
		path = FileExists(path) ? String("\"") + path + "\"" : String("git");
	}
	return path;
#else
	return "git";
#endif
}

bool IsGLSLExt(const String& ext)
{
	return findarg(ext, ".vert", ".tesc", ".tese", ".geom", ".frag", ".comp") >= 0;
}

String GetLocalDir()
{
	return ConfigFile("UppLocal");
}

String LocalPath(const String& filename)
{
	return AppendFileName(GetLocalDir(), filename);
}

Vector<String> SplitDirs(const char *s) {
#ifdef PLATFORM_POSIX
	return Split(s, [](int c) { return findarg(c, ';', ':') >= 0 ? c : 0; });
#else
	return Split(s, ';');
#endif
}

String GetAnyFileName(const char *path)
{
	const char *p = path;
	for(char c; (c = *path++) != 0;)
		if(c == '\\' || c == '/' || c == ':')
			p = path;
	return p;
}

String CatAnyPath(String path, const char *more)
{
	if(!more || !*more)
		return path;
	if(!path.IsEmpty() && *path.Last() != '\\' && *path.Last() != '/' &&
	*more != '\\' && *more != '/')
#ifdef PLATFORM_WIN32
		path.Cat('\\');
#else
		path.Cat('/');
#endif
	path.Cat(more);
	return path;
}

bool IsCSourceFile(const char *path)
{
	String ext = ToLower(GetFileExt(path));
	return ext == ".cpp" || ext == ".c" || ext == ".cc" || ext == ".cxx"
	                     || ext == ".m" || ext == ".mm" || ext == ".icpp";
}

bool IsCHeaderFile(const char *path)
{
	String ext = ToLower(GetFileExt(path));
	return ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx" || ext == ".i";
}

bool IsFullDirectory(const String& d) {
	if(IsFullPath(d)) {
		FindFile ff(d);
		if(ff) return ff.IsDirectory();
	}
	return false;
}

bool IsFolder(const String& path)
{
	if(IsNull(path) || *path.Last() == '\\' || *path.Last() == '/' || *path.Last() == ':')
		return true;
	if(path.Find('?') >= 0 || path.Find('*') >= 0)
		return false;
	FindFile ff(path);
	return ff && ff.IsDirectory();
}

int FindId(const String& s, const String& id)
{
	if(id.IsEmpty())
		return -1;

	int q = 0;
	for(;;) {
		q = s.Find(id, q);
		if(q < 0)
			return -1;
		bool before_ok = q == 0 || !iscid(s[q - 1]);
		bool after_ok = q + id.GetCount() >= s.GetCount() || !iscid(s[q + id.GetCount()]);
		if(before_ok && after_ok)
			return q;
		q++;
	}
}
