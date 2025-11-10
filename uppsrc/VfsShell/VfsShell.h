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
	virtual void ClearOutput() = 0;
	virtual void AddOutput(const String& s) = 0;
	virtual void AddOutputLine(const String& s = String()) = 0;
};

class VfsShellConsole {
	VfsShellHostBase& host;
	String            cwd;       // Current working directory (system path by default)
	String            line_header;
	ValueMap          vars;      // Shell variables
	
public:
	VfsShellConsole(VfsShellHostBase& h);
	
	void Execute();
	void PrintLineHeader();
	
	// Path operations
	bool SetCurrentDirectory(const String& path);
	const String& GetCurrentDirectory() const { return cwd; }
	
	// Check if path is in VFS overlay
	static bool IsVfsPath(const String& path);
	// Convert path to internal format (system or VFS)
	static String ConvertToInternalPath(const String& path, const String& cwd);
	// Convert internal path back to external format
	static String ConvertFromInternalPath(const String& path);
	
	// Command implementations
	void CmdHelp(const ValueArray& args);
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
	String output;  // For internal use (store command input/output)
	void ClearOutput() { host.ClearOutput(); }
	void AddOutput(const String& s) { host.AddOutput(s); }
	void AddOutputLine(const String& s = String()) { host.AddOutputLine(s); }
};

END_UPP_NAMESPACE

#endif
