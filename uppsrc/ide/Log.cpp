#ifdef flagGUI
#include "ide.h"

#define METHOD_NAME "Ide::" << UPP_FUNCTION_NAME << "(): "

String Ide::GetIdeLogPath()
{
	return GetStdLogPath();
}

String Ide::GetTargetLogPath()
{
	if(target.GetCount() == 0)
		return Null;
#ifdef PLATFORM_WIN32
	return ForceExt(target, ".log");
#else
	String title = GetFileTitle(target);
	String pp = GetFileFolder(GetUserConfigDir(GetFileFolder(target))) + "/.local/state/u++/log/" + title + ".log";
	if(FileExists(pp))
		return pp;
	String h = GetFileFolder(target);
	while(h.GetCount() > 1 && DirectoryExists(h)) {
		String p = AppendFileName(h, ".config");
		FindFile ff(p);
		if(ff && ff.IsFolder() && ff.CanWrite()) {
			pp = p;
			break;
		}
		h = GetFileFolder(h);
	}
	if(IsNull(pp))
		pp = GetEnv("XDG_CONFIG_HOME");
	if(IsNull(pp) || !DirectoryExists(pp))
		pp = GetHomeDirFile(".config");
	pp << "/u++/" << title << '/' << title << ".log";
	return FileExists(pp) ? pp : GetHomeDirFile(".upp/" + title + "/" + title + ".log");
#endif
}

void Ide::OpenLog(const String& logFilePath)
{
	String normalizedPath = NormalizePath(logFilePath);
	if(!designer && normalizedPath == editfile) {
		History(-1);
		return;
	}
	AddHistory();
	if(!FileExists(logFilePath)) {
		Loge() << METHOD_NAME << "Following log file doesn't exist: \"" << logFilePath << "\".";
		return;
	}
	
	EditFile(logFilePath);
}

bool Ide::IsInLogFile()
{
	return !designer && ToLower(editfile).EndsWith(".log");
}

bool Ide::FindLOG()
{
	if(!IsInLogFile())
		return false;

	String ln = editor.GetUtf8Line(editor.GetCursorLine());

	for(const char *s = ln; *s;) { // try to find source file path first
		if(*s == ' ' || *s == '\t') {
			while(*s == ' ' || *s == '\t') s++;
			if(*s)
				for(const char *q = s + 1; *q; q++) {
					String path = String(s, q);
					if(FileExists(path)) {
						while(*q) {
							if(IsDigit(*q)) {
								AddHistory();
								EditFile(path);
								editor.SetCursor(editor.GetPos(Atoi(q) - 1, 0));
								AddHistory();
								return true;
							}
							q++;
						}
						break;
					}
				}
		}
		else
			s++;
	}

	Vector<String> files = FindXFiles(3);

	int n = 0;
	Progress pi;
	struct LogLine : Moveable<LogLine> {
		String text;
		int    priority;
		String file;
		int    line;
	};
	Vector<LogLine> logs;
	for(String fn : files) {
		if(pi.SetCanceled(n++, files.GetCount()))
			break;

		if(GetFileLength(fn) < 10*1024*1024) {
			String text = LoadFile(fn);
			try {
				CParser p(text);
				while(!p.IsEof()) {
					CParser::Pos pos = p.GetPos();
					int priority = 1;
					auto AddLogLine = [&](const String& text) {
						auto& l = logs.Add();
						l.text = text;
						l.file = fn;
						l.line = pos.line;
						l.priority = priority;
					};
					auto SkipLogBody0 = [&] {
						int lvl = 1;
						while(!p.IsEof())
							if(p.Char('('))
								lvl++;
							else
							if(p.Char(')')) {
								if(--lvl == 0)
									break;
							}
							else
								p.Skip();
					};
					auto SkipLogBody = [&] {
						if(!p.Char('('))
							return;
						SkipLogBody0();
					};
					priority = p.IsId("DLOG") || p.IsId("DDUMP") || p.IsId("DTIMING") || p.IsId("DTIMESTOP") ? 2 :
					           p.IsId("LLOG") || p.IsId("LDUMP") || p.IsId("LTIMING") || p.IsId("LTIMESTOP") ? 0 : 1;
					if(p.Char('#'))
						p.SkipLine();
					else
					if(p.Id("TIMING") || p.Id("DTIMING") || p.Id("RTIMING") || p.Id("LTIMING")) {
					    p.Char('(');
						if(p.IsString())
							AddLogLine("TIMING " + p.ReadString());
						SkipLogBody0();
					}
					if(p.Id("LOG") || p.Id("DLOG") || p.Id("RLOG") || p.Id("LLOG") ||
					   p.Id("TIMESTOP") || p.Id("DTIMESTOP") || p.Id("RTIMESTOP") || p.Id("LTIMESTOP")) {
					    p.Char('(');
						if(p.IsString())
							AddLogLine(p.ReadString());
						SkipLogBody0();
					}
					else
					if(p.Id("DUMP") || p.Id("DDUMP") || p.Id("RDUMP") || p.Id("LDUMP")) {
						if(p.IsString()) {
							AddLogLine(p.ReadString());
							SkipLogBody();
						}
						else {
							const char *b = p.GetPtr();
							SkipLogBody();
							String text = String(b, p.GetPtr());
							text = TrimBoth(text);
							text.TrimStart("(");
							text.TrimEnd(")");
							AddLogLine(TrimBoth(text) + " = ");
						}
					}
					else
						p.Skip();
				}
			}
			catch(CParser::Error) {}
		}
	}

	ln = editor.GetUtf8Line(editor.GetCursorLine());

	LogLine bestl;
	int best = -1;
	int bestp = false;
	for(const LogLine& l : logs) {
		int ml = GetMatchLen(l.text, ln);
		if(ml > best || (ml == best && l.priority > bestp)) {
			best = ml;
			bestp = l.priority;
			bestl = l;
		}
	}

	if(bestl.file.GetCount()) {
		AddHistory();
		EditFile(bestl.file);
		editor.SetCursor(editor.GetPos(bestl.line - 1, 0));
		AddHistory();
	}
	return true;
}
#endif // flagGUI
