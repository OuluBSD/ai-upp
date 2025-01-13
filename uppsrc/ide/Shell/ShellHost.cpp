#include "Shell.h"
#include <Intranet/Intranet.h>

NAMESPACE_UPP

IdeShellHostBase::IdeShellHostBase() {
	
}

IdeShellHostBase::~IdeShellHostBase() {
	
}



IdeShellHost::IdeShellHost() {
	AddProgram("ls",	THISBACK(ListFiles));
	AddProgram("cd",	THISBACK(ChangeDirectory));
	
	#ifdef flagHAVE_INTRANET
	AddProgram("intra",	THISBACK(StartIntranet));
	#endif
}

const String& IdeShellHost::GetOutput() const {
	return out;
}

const String& IdeShellHost::GetError() const {
	return err;
}

void IdeShellHost::Put(const String& s) {
	out << s;
}

void IdeShellHost::PutLine(const String& s) {
	out << s << "\n";
}

void IdeShellHost::AddProgram(String cmd, Callback2<IdeShell&,Value> cb) {
	commands.Add(cmd, cb);
}

bool IdeShellHost::Command(IdeShell& shell, Value cmd) {
	ValueArray args;
	if (cmd.Is<ValueArray>()) args = cmd;
	else args.Add(cmd);
	if (args.IsEmpty())
		return false;
	String main_cmd = args[0].ToString();
	args.Remove(0);
	
	LOG("IdeShellHost::Command: " << args.ToString());
	
	String ret;
	
	out.Clear();
	err.Clear();
	
	for(int i = 0; i < commands.GetCount(); i++) {
		const String& c = commands.GetKey(i);
		if (main_cmd == c) {
			commands[i](shell, args);
			
			// Leave out last \n
			if (!out.IsEmpty() && *(out.End()-1) == '\n')
				out = out.Left(out.GetCount()-1);
			
			return true;
		}
	}
	
	return false;
}

String GetFirstValue(Value arg) {
	if (arg.Is<ValueArray>()) {
		ValueArray va = arg;
		if (va.GetCount() >= 1)
			return va[0];
		else
			return String();
	}
	return arg;
}

void IdeShellHost::ListFiles(IdeShell& shell, Value arg) {
	MountManager& mm = MountManager::System();
	String path_str = GetFirstValue(arg);
	if (path_str.IsEmpty())
		path_str = shell.cwd.str;
	else if (!IsFullDirectory(path_str))
		AppendUnixFileName(shell.cwd.str, path_str);
	VfsPath path(path_str);
	Vector<VfsItem> items;
	mm.GetFiles(path, items);
	const int cols = 4;
	const int col_width = 40;
	int col = 0;
	for (const VfsItem& i : items) {
		String n = i.name;
		if (n.GetCount() > col_width-4)
			n = n.Left(col_width-4) + "...";
		out << n;
		if (col < cols-1) {
			int spaces = col_width - n.GetCount();
			out.Cat(' ', spaces);
			col++;
		}
		else {
			col = 0;
		}
	}
}

void IdeShellHost::ChangeDirectory(IdeShell& shell, Value arg) {
	
}

#ifdef flagHAVE_INTRANET
void IdeShellHost::StartIntranet(IdeShell& shell, Value arg) {
	Thread::Start(IntranetDaemon);
}
#endif

END_UPP_NAMESPACE
