#ifndef _VfsShell_VfsShell_h_
#define _VfsShell_VfsShell_h_

#include <Core/Core.h>
#include <Core/VfsBase/VfsBase.h>  // Using Core/VfsBase instead of full Vfs package

NAMESPACE_UPP

// VfsShell - A command-line interface for VFS operations
class VfsShellHostBase {
public:
	virtual ~VfsShellHostBase() {}
	virtual bool Command(class VfsShellConsole& shell, const ValueArray& args) = 0;
	virtual String GetOutput() const = 0;
};

class VfsShellConsole {
	VfsShellHostBase& host;
	VfsPath           cwd;       // Current working directory in VFS
	String            line_header;
	ValueMap          vars;      // Shell variables
	
public:
	VfsShellConsole(VfsShellHostBase& h);
	
	void Execute();
	void PrintLineHeader();
	
	// VFS operations
	bool SetCurrentDirectory(const VfsPath& path);
	const VfsPath& GetCurrentDirectory() const { return cwd; }
	
	// Command implementations
	void CmdPwd();
	void CmdCd(const ValueArray& args);
	void CmdLs(const ValueArray& args);
	void CmdTree(const ValueArray& args);
	void CmdMkdir(const ValueArray& args);
	void CmdTouch(const ValueArray& args);
	void CmdRm(const ValueArray& args);
	void CmdMv(const ValueArray& args);
	void CmdLink(const ValueArray& args);
	void CmdExport(const ValueArray& args);
	void CmdCat(const ValueArray& args);
	void CmdGrep(const ValueArray& args);
	void CmdRg(const ValueArray& args);
	void CmdHead(const ValueArray& args);
	void CmdTail(const ValueArray& args);
	void CmdUniq(const ValueArray& args);
	void CmdCount(const ValueArray& args);
	void CmdHistory(const ValueArray& args);
	void CmdRandom(const ValueArray& args);
	void CmdTrueFalse(const ValueArray& args);
	void CmdEcho(const ValueArray& args);
	
private:
	String output;
	void ClearOutput() { output.Clear(); }
	void AddOutput(const String& s) { output << s; }
	void AddOutputLine(const String& s = String()) { output << s << "\n"; }
};

END_UPP_NAMESPACE

#endif
