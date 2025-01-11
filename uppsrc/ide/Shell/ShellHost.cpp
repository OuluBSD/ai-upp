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

void IdeShellHost::AddProgram(String cmd, Callback1<Value> cb) {
	commands.Add(cmd, cb);
}

bool IdeShellHost::Command(Value cmd) {
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
			commands[i](args);
			
			// Leave out last \n
			if (!out.IsEmpty() && *(out.End()-1) == '\n')
				out = out.Left(out.GetCount()-1);
			
			return true;
		}
	}
	
	return false;
}

void IdeShellHost::ListFiles(Value arg) {
	MountManager& mm = MountManager::System();
	
}

void IdeShellHost::ChangeDirectory(Value arg) {
	
}

#ifdef flagHAVE_INTRANET
void IdeShellHost::StartIntranet(Value arg) {
	Thread::Start(IntranetDaemon);
}
#endif

END_UPP_NAMESPACE
