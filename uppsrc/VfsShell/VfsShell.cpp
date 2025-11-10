#include "VfsShell.h"
#include <Core/VfsBase/VfsBase.h>  // For basic VFS operations

NAMESPACE_UPP

// Check if path is in VFS overlay
bool VfsShellConsole::IsVfsPath(const String& path) {
	return path.StartsWith("/vfs/");
}

// Convert path to internal format (system or VFS)
String VfsShellConsole::ConvertToInternalPath(const String& path, const String& cwd) {
	if (path.IsEmpty()) return cwd;
	
	// If it's already in the VFS overlay, return as is
	if (IsVfsPath(path)) {
		return path;
	}
	
	// If it's an absolute system path, return as is
	if (path.StartsWith("/")) {
		return path;
	}
	
	// If it's the VFS root
	if (path == "/vfs") {
		return "/vfs/";
	}
	
	// Handle relative paths by resolving against current working directory
	String newPath;
	if (cwd == "/vfs") {
		newPath = "/vfs/";
	} else {
		newPath = cwd;
	}
	
	if (!newPath.EndsWith("/")) newPath += "/";
	newPath += path;
	
	// Normalize the path (handle "..", ".", etc.)
	// We'll use U++'s path utilities where possible
	return NormalizePath(newPath);
}

// Convert internal path back to external format
String VfsShellConsole::ConvertFromInternalPath(const String& path) {
	return path; // For now, just return as is
}

VfsShellConsole::VfsShellConsole(VfsShellHostBase& h) : host(h)
{
	cwd = "/"; // Start in the root directory where system filesystem is mounted
}

void VfsShellConsole::CmdHelp(const ValueArray& args) {
	String helpText = 
		"Available commands:\n"
		"  help          - Show this help message\n"
		"  pwd           - Print current working directory\n"
		"  cd [path]     - Change current directory\n"
		"  ls [path]     - List directory contents\n"
		"  tree [path]   - Show directory tree structure\n"
		"  mkdir <path>  - Create a new directory\n"
		"  touch <path>  - Create or update a file\n"
		"  rm <path>     - Remove a file or directory\n"
		"  mv <src> <dst> - Move/rename a file or directory\n"
		"  link <src> <dst> - Create a link\n"
		"  export <vfs> <host> - Export VFS content to host filesystem\n"
		"  cat [paths...] - Concatenate and display file contents\n"
		"  grep [-i] <pattern> [path] - Search for pattern in files\n"
		"  rg [-i] <pattern> [path] - Ripgrep-like search\n"
		"  head [-n N] [path] - Show first N lines of file\n"
		"  tail [-n N] [path] - Show last N lines of file\n"
		"  uniq [path]   - Show unique lines in file\n"
		"  count [path]  - Count lines in file\n"
		"  history [-a | -n N] - Show command history\n"
		"  random [min [max]] - Generate random number\n"
		"  true/false    - Exit with success/failure status\n"
		"  echo <path> <data...> - Display data\n"
		"  quit/exit     - Exit the shell\n";
	AddOutputLine(helpText);
}

void VfsShellConsole::Execute()
{
	// For console implementation, we'll assume a simple command input
	// In a real console application, we'd read from stdin, but for this integration
	// with the existing framework we'll work with the stored output
	String txt;
	
	// Process the last command stored in output
	String s = output;
	if (!line_header.IsEmpty() && s.StartsWith(line_header))
		s = s.Mid(line_header.GetCount());  // Remove prompt from line
	s = TrimBoth(s);

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

	// Output result
	if (!txt.IsEmpty()) {
		AddOutputLine(txt);
	}
	// Print new prompt
	AddOutputLine();
	PrintLineHeader();
}

void VfsShellConsole::PrintLineHeader() {
	String user = GetUserName();  // Get actual username
	if (user.IsEmpty()) user = "user";
	String host_name = "vfsshell";  // Simple host name
	String s = user + "@" + host_name + ":" + cwd + " $ ";
	line_header = s;
	AddOutput(s);
}



bool VfsShellConsole::SetCurrentDirectory(const String& path) {
	String normalizedPath = ConvertToInternalPath(path, cwd);
	
	// Check if this is a VFS path
	if (IsVfsPath(normalizedPath)) {
		// Convert to VfsPath for VFS operations
		VfsPath vfsPath;
		vfsPath.Set(normalizedPath);
		
		MountManager& mm = MountManager::System();
		if (!mm.DirectoryExists(vfsPath))
			return false;
		cwd = normalizedPath;
		return true;
	} else {
		// This is a system path, check if directory exists using the mount system
		VfsPath vfsPath;
		vfsPath.Set(normalizedPath);

		MountManager& mm = MountManager::System();
		if (!mm.DirectoryExists(vfsPath))
			return false;
		cwd = normalizedPath;
		return true;
	}
}

// Implementation of pwd command
void VfsShellConsole::CmdPwd() {
	AddOutputLine(GetCurrentDirectory());
}

// Implementation of cd command
void VfsShellConsole::CmdCd(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("cd: missing directory operand");
		return;
	}
	
	String targetPath = args[1];
	
	if (SetCurrentDirectory(targetPath)) {
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
		targetPath = GetCurrentDirectory();
	}
	
	String resolvedPath = ConvertToInternalPath(targetPath, GetCurrentDirectory());

	// Check if this is a VFS path
	if (IsVfsPath(resolvedPath)) {
		// Convert to VfsPath for VFS operations
		VfsPath vfsPath;
		vfsPath.Set(resolvedPath);

		MountManager& mm = MountManager::System();
		Vector<VfsItem> items;
		if (mm.GetFiles(vfsPath, items)) {
			for (const VfsItem& item : items) {
				AddOutputLine(item.name);
			}
		} else {
			AddOutputLine("ls: cannot access '" + resolvedPath + "': " + mm.last_error);
		}
	} else {
		// This is a system path
		if (DirectoryExists(resolvedPath)) {
			FindFile ff;
			if (ff.Search(AppendFileName(resolvedPath, "*"))) {
				do {
					String name = ff.GetName();
					if (name == "." || name == "..") continue;
					AddOutputLine(name);
				} while (ff.Next());
			}
		} else {
			AddOutputLine("ls: cannot access '" + resolvedPath + "': No such file or directory");
		}
	}
}

// Implementation of tree command
void VfsShellConsole::CmdTree(const ValueArray& args) {
	String targetPath;
	if (args.GetCount() > 1) {
		targetPath = args[1];
	} else {
		targetPath = GetCurrentDirectory();
	}
	
	String resolvedPath = ConvertToInternalPath(targetPath, GetCurrentDirectory());

	// Check if this is a VFS path
	if (IsVfsPath(resolvedPath)) {
		// Convert to VfsPath for VFS operations
		VfsPath vfsPath;
		vfsPath.Set(resolvedPath);

		MountManager& mm = MountManager::System();
		Vector<VfsItem> items;
		if (mm.GetFiles(vfsPath, items)) {
			AddOutputLine(resolvedPath + "/");
			for (const VfsItem& item : items) {
				AddOutputLine("├── " + item.name);
			}
		} else {
			AddOutputLine("tree: cannot access '" + resolvedPath + "': " + mm.last_error);
		}
	} else {
		// This is a system path, check using MountManager
		VfsPath vfsPath;
		vfsPath.Set(resolvedPath);

		MountManager& mm = MountManager::System();
		Vector<VfsItem> items;
		if (mm.GetFiles(vfsPath, items)) {
			AddOutputLine(resolvedPath + "/");
			for (const VfsItem& item : items) {
				AddOutputLine("├── " + item.name);
			}
		} else {
			AddOutputLine("tree: cannot access '" + resolvedPath + "': " + mm.last_error);
		}
	}
}

// Implementation of mkdir command
void VfsShellConsole::CmdMkdir(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("mkdir: missing operand");
		return;
	}
	
	String targetPath = args[1];
	String resolvedPath = ConvertToInternalPath(targetPath, GetCurrentDirectory());

	// Check if this is a VFS path
	if (IsVfsPath(resolvedPath)) {
		// Convert to VfsPath for VFS operations
		VfsPath vfsPath;
		vfsPath.Set(resolvedPath);

		// Note: The current VFS implementation doesn't have a CreateDirectory method
		// This would require an actual implementation in the underlying VFS system
		AddOutputLine("mkdir: command not fully implemented in VFS - underlying VFS doesn't support directory creation yet");
	} else {
		// This is a system path - for now, use system calls directly as MountManager doesn't have CreateDirectory
		if (RealizeDirectory(resolvedPath)) {
			// Directory created successfully
		} else {
			AddOutputLine("mkdir: cannot create directory '" + resolvedPath + "': " + GetLastErrorMessage());
		}
	}
}

// Implementation of touch command
void VfsShellConsole::CmdTouch(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("touch: missing file operand");
		return;
	}
	
	String targetPath = args[1];
	String resolvedPath = ConvertToInternalPath(targetPath, GetCurrentDirectory());

	// Check if this is a VFS path
	if (IsVfsPath(resolvedPath)) {
		// Convert to VfsPath for VFS operations
		VfsPath vfsPath;
		vfsPath.Set(resolvedPath);

		MountManager& mm = MountManager::System();

		// Check if file exists
		if (!mm.FileExists(vfsPath) && !mm.DirectoryExists(vfsPath)) {
			// Note: The current VFS implementation doesn't support creating files directly
			// This would require an actual implementation in the underlying VFS system
			AddOutputLine("touch: command not fully implemented in VFS - underlying VFS doesn't support file creation yet");
		}
		// If it exists, we could potentially update timestamp, but that's not implemented yet
	} else {
		// This is a system path
		if (!FileExists(resolvedPath) && !DirectoryExists(resolvedPath)) {
			// Create the file if it doesn't exist
			FileOut out(resolvedPath);
			if (!out.IsOpen()) {
				AddOutputLine("touch: cannot touch '" + resolvedPath + "': " + GetLastErrorMessage());
				return;
			}
		} else {
			// Update timestamp if it exists (not directly supported in U++, so just confirm existence)
		}
	}
}

// Implementation of rm command
void VfsShellConsole::CmdRm(const ValueArray& args) {
	if (args.GetCount() < 2) {
		AddOutputLine("rm: missing file operand");
		return;
	}
	
	String targetPath = args[1];
	VfsPath path;
	if (targetPath.StartsWith("/")) {
		path.Set(targetPath);
	} else {
		VfsPath relPath;
		relPath.Set(targetPath);
		VfsPath cwdPath;
		cwdPath.Set(cwd);
		path = cwdPath / relPath;
	}
	
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
	
	VfsPath srcPath;
	if (srcPathStr.StartsWith("/")) {
		srcPath.Set(srcPathStr);
	} else {
		VfsPath srcRelPath;
		srcRelPath.Set(srcPathStr);
		VfsPath cwdPath; cwdPath.Set(cwd); srcPath = cwdPath / srcRelPath;
	}
	
	VfsPath dstPath;
	if (dstPathStr.StartsWith("/")) {
		dstPath.Set(dstPathStr);
	} else {
		VfsPath dstRelPath;
		dstRelPath.Set(dstPathStr);
		VfsPath cwdPath2; cwdPath2.Set(cwd); dstPath = cwdPath2 / dstRelPath;
	}
	
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
	
	VfsPath srcPath;
	if (srcPathStr.StartsWith("/")) {
		srcPath.Set(srcPathStr);
	} else {
		VfsPath srcRelPath;
		srcRelPath.Set(srcPathStr);
		VfsPath cwdPath3; cwdPath3.Set(cwd); srcPath = cwdPath3 / srcRelPath;
	}
	
	VfsPath dstPath;
	if (dstPathStr.StartsWith("/")) {
		dstPath.Set(dstPathStr);
	} else {
		VfsPath dstRelPath;
		dstRelPath.Set(dstPathStr);
		VfsPath cwdPath4; cwdPath4.Set(cwd); dstPath = cwdPath4 / dstRelPath;
	}
	
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
	
	VfsPath vfsPath;
	if (vfsPathStr.StartsWith("/")) {
		vfsPath.Set(vfsPathStr);
	} else {
		VfsPath relPath;
		relPath.Set(vfsPathStr);
		VfsPath cwdPath5; cwdPath5.Set(cwd); vfsPath = cwdPath5 / relPath;
	}
	
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
		VfsPath path;
		if (pathStr.StartsWith("/")) {
			path.Set(pathStr);
		} else {
			VfsPath relPath;
			relPath.Set(pathStr);
			VfsPath cwdPath6; cwdPath6.Set(cwd); path = cwdPath6 / relPath;
		}
		
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
		lineCount = StrInt((String)args[2]);
		argStart = 3;
	}
	
	if (args.GetCount() <= argStart) {
		AddOutputLine("head: missing file operand");
		return;
	}
	
	String pathStr = args[argStart];
	VfsPath path;
	if (pathStr.StartsWith("/")) {
		path.Set(pathStr);
	} else {
		VfsPath relPath;
		relPath.Set(pathStr);
		VfsPath cwdPath7; cwdPath7.Set(cwd); path = cwdPath7 / relPath;
	}
	
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
		lineCount = StrInt((String)args[2]);
		argStart = 3;
	}
	
	if (args.GetCount() <= argStart) {
		AddOutputLine("tail: missing file operand");
		return;
	}
	
	String pathStr = args[argStart];
	VfsPath path;
	if (pathStr.StartsWith("/")) {
		path.Set(pathStr);
	} else {
		VfsPath relPath;
		relPath.Set(pathStr);
		VfsPath cwdPath8; cwdPath8.Set(cwd); path = cwdPath8 / relPath;
	}
	
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
	VfsPath path;
	if (pathStr.StartsWith("/")) {
		path.Set(pathStr);
	} else {
		VfsPath relPath;
		relPath.Set(pathStr);
		VfsPath cwdPath9; cwdPath9.Set(cwd); path = cwdPath9 / relPath;
	}
	
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
	VfsPath path;
	if (pathStr.StartsWith("/")) {
		path.Set(pathStr);
	} else {
		VfsPath relPath;
		relPath.Set(pathStr);
		VfsPath cwdPath10; cwdPath10.Set(cwd); path = cwdPath10 / relPath;
	}
	
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
		minVal = StrInt((String)args[1]);
		if (args.GetCount() > 2) {
			maxVal = StrInt((String)args[2]);
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
		result.Cat((String)args[i]);
	}
	AddOutputLine(result);
}

END_UPP_NAMESPACE