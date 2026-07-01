#include "Core.h"

#ifdef PLATFORM_WIN32
#include <wincon.h>
#endif

Mutex                     s_allfiles_lock;
VectorMap<String, String> s_allfiles;
Vector<String>            s_allnests;

void ForAllSourceFiles(Event<const VectorMap<String, String>&> fn)
{
	Mutex::Lock __(s_allfiles_lock);
	fn(s_allfiles);
}

void ForAllNests(Event<const Vector<String>&> fn)
{
	Mutex::Lock __(s_allfiles_lock);
	fn(s_allnests);
}

void SetAllSourceFiles(VectorMap<String, String> files, Vector<String> nests)
{
	Mutex::Lock __(s_allfiles_lock);
	s_allfiles = pick(files);
	s_allnests = pick(nests);
}

Index<String> GetAllNests(bool sleep)
{
	Index<String> dir;
	for(FindFile ff(ConfigFile("*.var")); ff && !Thread::IsShutdownThreads(); ff.Next()) {
		VectorMap<String, String> var;
		LoadVarFile(ff.GetPath(), var);
		for(String d : Split(var.Get("UPP", ""), ';'))
			dir.FindAdd(NormalizePath(d));
		if(sleep)
			Sleep(0);
	}
	return dir;
}

void DelTemps()
{
	FindFile ff(ConfigFile("*.tmp"));
	while(ff) {
		DeleteFile(ConfigFile(ff.GetName()));
		ff.Next();
	}
}

bool SilentMode;

#ifdef PLATFORM_WIN32
void Puts(const char *s)
{
	dword dummy;
	if(!SilentMode)
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, (int)strlen(s), &dummy, NULL);
}
#endif

#ifdef PLATFORM_POSIX
void Puts(const char *s)
{
	if(!SilentMode)
		puts(s);
}
#endif

int CommaSpace(int c)
{
	return c == ',' ? ' ' : c;
}

void ReduceCfgCache()
{
	String cfgdir = ConfigFile("cfg");
	FindFile ff(AppendFileName(cfgdir, "*.*"));
	while(ff) {
		if(ff.IsFile()) {
			String fn = ff.GetName();
			String ext = GetFileExt(fn);
			if(ext != ".aux" && ext != ".cfg")
				if((Date)Time(ff.GetLastAccessTime()) < GetSysDate() - 14)
					DeleteFile(AppendFileName(cfgdir, fn));
		}
		ff.Next();
	}
}

bool IsAssembly(const String& s)
{
	for(FindFile ff(ConfigFile("*.var")); ff; ff.Next())
		if(ff.IsFile())
			if(GetFileTitle(ff.GetName()) == s)
				return true;
	Vector<String> l = Split(s, ',');
	for(int i = 0; i < l.GetCount(); i++)
		if(FindFile(NormalizePath(l[i])).IsFolder())
			return true;
	return false;
}
