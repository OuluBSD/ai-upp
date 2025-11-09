#include "VfsShell.h"
#include <Core/VfsBase/VfsBase.h>  // For basic VFS operations

NAMESPACE_UPP

VfsShellConsole::VfsShellConsole(VfsShellHostBase& h) : host(h)
{
	cwd.Set(INTERNAL_ROOT_PATH);  // Use internal root as starting point
	NoHorzScrollbar();
	
	// Initialize with a prompt
	PrintLineHeader();
}

void VfsShellConsole::Execute()
{
	int li = GetLineCount() - 1;
	if(IsSelection()) {
		String s = GetSelection();
		if(s.GetLength() < 80) {
			SetCursor(GetLength());
			Paste(s.ToWString());  // Just paste selection to command line
		}
		return;
	}
	if(GetLine(GetCursor()) != li) {
		WString s = GetWLine(GetLine(GetCursor()));
		if(s[0] == '$')  // If line starts with $, remove it
			s = s.Mid(1);
		SetCursor(GetLength());
		Paste(s);
		return;
	}

	String txt;
	String src_line = GetUtf8Line(li);
	if (!line_header.IsEmpty())
		src_line = src_line.Mid(line_header.GetCount());  // Remove prompt from line
	String s = TrimBoth(src_line);

	if (s.IsEmpty()) return;

	bool succ = false;
	
	// Try to parse and execute command
	try {
		Vector<String> parts = Split(s, " ", true, true);  // Split command line into parts
		if (parts.GetCount() > 0) {
			ValueArray args;
			for (const String& part : parts) {
				args.Add(part);
			}
			
			if (host.Command(*this, args)) {  // Let host handle command
				succ = true;
				txt = host.GetOutput();
			}
		}
	}
	catch (Exc e) {
		txt << "ERROR: " << e;
	}

	if (!succ) {
		txt = "Unknown command: " + s;
	}

	// Add the command to history
	AddHistory(s);

	// Output result
	if (!txt.IsEmpty()) {
		Paste("\n");
		Paste(txt.ToWString());
	}
	Paste("\n");
	PrintLineHeader();  // Print new prompt
}

void VfsShellConsole::PrintLineHeader() {
	String user = GetUserName();  // Get actual username
	if (user.IsEmpty()) user = "user";
	String host_name = "vfsshell";  // Simple host name
	String s = user + "@" + host_name + ":" + (String)cwd + " $ ";
	line_header = s;
	Paste(line_header.ToWString());
}

void VfsShellConsole::LeftDouble(Point p, dword flags)
{
	CodeEditor::LeftDouble(p, flags);
	if(IsSelection())
		Execute();
}

bool VfsShellConsole::Key(dword key, int count)
{
	switch(key) {
	case K_ENTER:
		Execute();
		break;
	case K_ALT_F7:  // Alt+F7 for context menu
	case K_F12:     // F12 for context menu
		break;
	default:
		return CodeEditor::Key(key, count);
	}
	return true;
}

bool VfsShellConsole::SetCurrentDirectory(const VfsPath& path) {
	MountManager& mm = MountManager::System();
	if (!mm.DirectoryExists(path))
		return false;
	cwd = path;
	return true;
}

// Implementation of pwd command
void VfsShellConsole::CmdPwd() {
	AddOutputLine((String)GetCurrentDirectory());
}

// Implementation of cd command
void VfsShellConsole::CmdCd(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("cd: missing directory operand");
		return;
	}
	
	String targetPath = args[1];
	VfsPath path = cwd;
	
	// Handle absolute vs relative paths
	if (targetPath.StartsWith("/")) {
		path.Set(targetPath);
	} else {
		path = cwd / targetPath;
	}
	
	if (SetCurrentDirectory(path)) {
		// Update prompt to reflect new directory
		ClearOutput(); // Clear any previous output
	} else {
		AddOutputLine("cd: no such file or directory: " + targetPath);
	}
}

// Implementation of ls command
void VfsShellConsole::CmdLs(const ValueArray& args) {
	String targetPath;
	if (args.GetCount() > 1) {
		targetPath = args[1];
	} else {
		targetPath = (String)GetCurrentDirectory();
	}
	
	VfsPath path = targetPath.StartsWith("/") ? VfsPath(targetPath) : cwd / targetPath;
	
	MountManager& mm = MountManager::System();
	Vector<VfsItem> items;
	if (mm.GetFiles(path, items)) {
		for (const VfsItem& item : items) {
			AddOutputLine(item.name);
		}
	} else {
		AddOutputLine("ls: cannot access '" + targetPath + "': " + mm.last_error);
	}
}

// Implementation of tree command
void VfsShellConsole::CmdTree(const ValueArray& args) {
	String targetPath;
	if (args.GetCount() > 1) {
		targetPath = args[1];
	} else {
		targetPath = (String)GetCurrentDirectory();
	}
	
	VfsPath path = targetPath.StartsWith("/") ? VfsPath(targetPath) : cwd / targetPath;
	
	MountManager& mm = MountManager::System();
	Vector<VfsItem> items;
	if (mm.GetFiles(path, items)) {
		AddOutputLine((String)path + "/");
		for (const VfsItem& item : items) {
			AddOutputLine("├── " + item.name);
		}
	} else {
		AddOutputLine("tree: cannot access '" + targetPath + "': " + mm.last_error);
	}
}

// Implementation of mkdir command
void VfsShellConsole::CmdMkdir(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("mkdir: missing operand");
		return;
	}
	
	String targetPath = args[1];
	VfsPath path = targetPath.StartsWith("/") ? VfsPath(targetPath) : cwd / targetPath;
	
	// Note: The current VFS implementation doesn't have a CreateDirectory method
	// This would require an actual implementation in the underlying VFS system
	AddOutputLine("mkdir: command not fully implemented - underlying VFS doesn't support directory creation yet");
}

// Implementation of touch command
void VfsShellConsole::CmdTouch(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("touch: missing file operand");
		return;
	}
	
	String targetPath = args[1];
	VfsPath path = targetPath.StartsWith("/") ? VfsPath(targetPath) : cwd / targetPath;
	
	MountManager& mm = MountManager::System();
	
	// Check if file exists
	if (!mm.FileExists(path) && !mm.DirectoryExists(path)) {
		// Note: The current VFS implementation doesn't support creating files directly
		// This would require an actual implementation in the underlying VFS system
		AddOutputLine("touch: command not fully implemented - underlying VFS doesn't support file creation yet");
	}
	// If it exists, we could potentially update timestamp, but that's not implemented yet
}

// Implementation of rm command
void VfsShellConsole::CmdRm(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("rm: missing file operand");
		return;
	}
	
	String targetPath = args[1];
	VfsPath path = targetPath.StartsWith("/") ? VfsPath(targetPath) : cwd / targetPath;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(path) || mm.DirectoryExists(path)) {
		// Note: The current VFS implementation doesn't have DeleteFile/DeleteDirectory methods
		// This would require an actual implementation in the underlying VFS system
		AddOutputLine("rm: command not fully implemented - underlying VFS doesn't support file/directory deletion yet");
	} else {
		AddOutputLine("rm: cannot remove '" + targetPath + "': No such file or directory");
	}
}

// Implementation of mv command
void VfsShellConsole::CmdMv(const ValueArray& args) {
	if (args.GetCount() < 3) {
		AddOutputLine("mv: missing destination file operand after source");
		return;
	}
	
	String srcPathStr = args[1];
	String dstPathStr = args[2];
	
	VfsPath srcPath = srcPathStr.StartsWith("/") ? VfsPath(srcPathStr) : cwd / srcPathStr;
	VfsPath dstPath = dstPathStr.StartsWith("/") ? VfsPath(dstPathStr) : cwd / dstPathStr;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(srcPath) || mm.DirectoryExists(srcPath)) {
		// Note: The current VFS implementation doesn't have a Rename method
		// This would require an actual implementation in the underlying VFS system
		AddOutputLine("mv: command not fully implemented - underlying VFS doesn't support file/directory renaming yet");
	} else {
		AddOutputLine("mv: cannot stat '" + srcPathStr + "': No such file or directory");
	}
}

// Implementation of link command
void VfsShellConsole::CmdLink(const ValueArray& args) {
	if (args.GetCount() < 3) {
		AddOutputLine("link: missing destination file operand after source");
		return;
	}
	
	String srcPathStr = args[1];
	String dstPathStr = args[2];
	
	VfsPath srcPath = srcPathStr.StartsWith("/") ? VfsPath(srcPathStr) : cwd / srcPathStr;
	VfsPath dstPath = dstPathStr.StartsWith("/") ? VfsPath(dstPathStr) : cwd / dstPathStr;
	
	// Note: The current VFS implementation doesn't have link functionality
	// This would require an actual implementation in the underlying VFS system
	AddOutputLine("link: command not fully implemented - underlying VFS doesn't support linking yet");
}

// Implementation of export command
void VfsShellConsole::CmdExport(const ValueArray& args) {
	if (args.GetCount() < 3) {
		AddOutputLine("export: missing destination path");
		return;
	}
	
	String vfsPathStr = args[1];
	String hostPathStr = args[2];
	
	VfsPath vfsPath = vfsPathStr.StartsWith("/") ? VfsPath(vfsPathStr) : cwd / vfsPathStr;
	
	// Note: The current VFS implementation doesn't have a MapPath method
	// This would require an actual implementation of a filesystem backend
	MountManager& mm = MountManager::System();
	// We cannot directly access VFS content in the current implementation
	AddOutputLine("export: command not fully implemented - underlying VFS doesn't support direct file export yet");
}

// Implementation of cat command
void VfsShellConsole::CmdCat(const ValueArray& args) {
	if (args.GetCount() < 2) {
		// If no file args, cat should typically read from stdin until EOF
		// For now, just a placeholder
		AddOutputLine("cat: usage: cat [path...]");
		return;
	}
	
	MountManager& mm = MountManager::System();
	
	for (int i = 1; i < args.GetCount(); i++) {
		String pathStr = args[i];
		VfsPath path = pathStr.StartsWith("/") ? VfsPath(pathStr) : cwd / pathStr;
		
		if (mm.FileExists(path)) {
			// Note: The current VFS implementation doesn't allow direct file reading
			// This would require an actual implementation of data access methods
			AddOutputLine("cat: command not fully implemented - underlying VFS doesn't support direct file content access yet");
		} else {
			AddOutputLine("cat: " + pathStr + ": No such file or directory");
		}
	}
}

// Implementation of grep command
void VfsShellConsole::CmdGrep(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("grep: usage: grep [-i] <pattern> [path]");
		return;
	}
	
	int argStart = 1;
	bool caseInsensitive = false;
	
	if (args[1] == "-i") {
		caseInsensitive = true;
		argStart = 2;
		if (args.GetCount() < 3) {
			AddOutputLine("grep: missing pattern");
			return;
		}
	}
	
	String pattern = args[argStart];
	String targetPath;
	if (args.GetCount() > argStart + 1) {
		targetPath = args[argStart + 1];
	} else {
		// Default to current directory if no path specified
		targetPath = (String)GetCurrentDirectory();
	}
	
	// Implementation would grep through file content
	AddOutputLine("grep: command not fully implemented yet for VFS paths");
}

// Implementation of rg command (ripgrep-like)
void VfsShellConsole::CmdRg(const ValueArray& args) {
	// For now, treat it similarly to grep
	if (args.GetCount() < 2) {
		AddOutputLine("rg: usage: rg [-i] <pattern> [path]");
		return;
	}
	
	AddOutputLine("rg: ripgrep functionality not fully implemented yet for VFS paths");
}

// Implementation of head command
void VfsShellConsole::CmdHead(const ValueArray& args) {
	int lineCount = 10; // Default
	int argStart = 1;
	
	if (args.GetCount() > 1 && args[1] == "-n") {
		if (args.GetCount() < 3) {
			AddOutputLine("head: option requires an argument -- 'n'");
			return;
		}
		lineCount = StrInt(args[2]);
		argStart = 3;
	}
	
	if (args.GetCount() <= argStart) {
		AddOutputLine("head: missing file operand");
		return;
	}
	
	String pathStr = args[argStart];
	VfsPath path = pathStr.StartsWith("/") ? VfsPath(pathStr) : cwd / pathStr;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(path)) {
		// Note: The current VFS implementation doesn't allow direct file reading
		// This would require an actual implementation of data access methods
		AddOutputLine("head: command not fully implemented - underlying VFS doesn't support direct file content access yet");
	} else {
		AddOutputLine("head: " + pathStr + ": No such file or directory");
	}
}

// Implementation of tail command
void VfsShellConsole::CmdTail(const ValueArray& args) {
	int lineCount = 10; // Default
	int argStart = 1;
	
	if (args.GetCount() > 1 && args[1] == "-n") {
		if (args.GetCount() < 3) {
			AddOutputLine("tail: option requires an argument -- 'n'");
			return;
		}
		lineCount = StrInt(args[2]);
		argStart = 3;
	}
	
	if (args.GetCount() <= argStart) {
		AddOutputLine("tail: missing file operand");
		return;
	}
	
	String pathStr = args[argStart];
	VfsPath path = pathStr.StartsWith("/") ? VfsPath(pathStr) : cwd / pathStr;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(path)) {
		// Note: The current VFS implementation doesn't allow direct file reading
		// This would require an actual implementation of data access methods
		AddOutputLine("tail: command not fully implemented - underlying VFS doesn't support direct file content access yet");
	} else {
		AddOutputLine("tail: " + pathStr + ": No such file or directory");
	}
}

// Implementation of uniq command
void VfsShellConsole::CmdUniq(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("uniq: missing input file");
		return;
	}
	
	String pathStr = args[1];
	VfsPath path = pathStr.StartsWith("/") ? VfsPath(pathStr) : cwd / pathStr;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(path)) {
		// Note: The current VFS implementation doesn't allow direct file reading
		// This would require an actual implementation of data access methods
		AddOutputLine("uniq: command not fully implemented - underlying VFS doesn't support direct file content access yet");
	} else {
		AddOutputLine("uniq: " + pathStr + ": No such file or directory");
	}
}

// Implementation of count command
void VfsShellConsole::CmdCount(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("count: missing input file");
		return;
	}
	
	String pathStr = args[1];
	VfsPath path = pathStr.StartsWith("/") ? VfsPath(pathStr) : cwd / pathStr;
	
	MountManager& mm = MountManager::System();
	if (mm.FileExists(path)) {
		// Note: The current VFS implementation doesn't allow direct file reading
		// This would require an actual implementation of data access methods
		AddOutputLine("count: command not fully implemented - underlying VFS doesn't support direct file content access yet");
	} else {
		AddOutputLine("count: " + pathStr + ": No such file or directory");
	}
}

// Implementation of history command
void VfsShellConsole::CmdHistory(const ValueArray& args) {
	AddOutputLine("history: command history functionality not fully implemented yet");
}

// Implementation of random command
void VfsShellConsole::CmdRandom(const ValueArray& args) {
	int minVal = 0;
	int maxVal = 100;
	
	if (args.GetCount() > 1) {
		minVal = StrInt(args[1]);
		if (args.GetCount() > 2) {
			maxVal = StrInt(args[2]);
		} else {
			maxVal = minVal;
			minVal = 0;
		}
	}
	
	if (minVal > maxVal) {
		int temp = minVal;
		minVal = maxVal;
		maxVal = temp;
	}
	
	int result = minVal + (Random() % (maxVal - minVal + 1));
	AddOutputLine(IntStr(result));
}

// Implementation of true/false commands
void VfsShellConsole::CmdTrueFalse(const ValueArray& args) {
	if (args.GetCount() < 1) return;
	
	String cmd = args[0];
	if (cmd == "true") {
		// Simply return success (no output)
	} else if (cmd == "false") {
		// For now, just show it was called - actual exit status isn't implemented here
		AddOutputLine("Command was 'false' - would normally exit with status 1");
	}
}

// Implementation of echo command
void VfsShellConsole::CmdEcho(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine(""); // Echo newline if no args
		return;
	}
	
	String result;
	for (int i = 1; i < args.GetCount(); i++) {
		if (i > 1) result.Cat(" ");
		result.Cat(args[i]);
	}
	AddOutputLine(result);
}

END_UPP_NAMESPACE