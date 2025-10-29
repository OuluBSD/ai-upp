#include "InternalShell.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

NAMESPACE_UPP

InternalShell::InternalShell() {
    prompt = "$ ";
    current_directory = GetHomeDirectory();
}

InternalShell::~InternalShell() {
}

String InternalShell::ExecuteCommand(const String& cmd) {
    if (cmd.IsEmpty()) return String();
    
    Vector<String> parts = ParseCommand(cmd);
    if (parts.GetCount() == 0) return String();
    
    String command = parts[0];
    String result;
    
    if (command == "cd") {
        result = CmdCd(parts);
    }
    else if (command == "pwd") {
        result = CmdPwd(parts);
    }
    else if (command == "ls") {
        result = CmdLs(parts);
    }
    else if (command == "echo") {
        result = CmdEcho(parts);
    }
    else if (command == "help") {
        result = CmdHelp(parts);
    }
    else if (command == "history") {
        result = CmdHistory(parts);
    }
    else if (command == "clear") {
        result = CmdClear(parts);
    }
    else {
        result = "Command not found: " + command + "\n";
    }
    
    command_history.Add(cmd);
    if (command_history.GetCount() > 100) {  // Limit history size
        command_history.Remove(0);
    }
    
    return result;
}

String InternalShell::ProcessInput(const String& input) {
    return ExecuteCommand(input);
}

String InternalShell::CmdCd(const Vector<String>& args) {
    if (args.GetCount() < 2) {
        current_directory = GetHomeDirectory();
    } else {
        String new_dir = args[1];
        if (new_dir == "~") {
            current_directory = GetHomeDirectory();
        } else if (new_dir.StartsWith("~/") || new_dir.StartsWith("~")) {
            String home = GetHomeDirectory();
            String relative_path = new_dir.Mid(2);
            current_directory = AppendFileName(home, relative_path);
        } else if (new_dir.StartsWith("/")) {
            current_directory = new_dir;
        } else {
            current_directory = AppendFileName(current_directory, new_dir);
        }
        
        // Check if directory exists
        struct stat info;
        if (stat(~current_directory, &info) != 0 || !S_ISDIR(info.st_mode)) {
            current_directory = GetHomeDirectory(); // fallback to home
            return "cd: no such file or directory: " + args[1] + "\n";
        }
    }
    return String();
}

String InternalShell::CmdPwd(const Vector<String>& args) {
    return current_directory + "\n";
}

String InternalShell::CmdLs(const Vector<String>& args) {
    String dir = current_directory;
    if (args.GetCount() > 1) {
        if (args[1].StartsWith("/")) {
            dir = args[1];
        } else {
            dir = AppendFileName(current_directory, args[1]);
        }
    }
    
    DIR* dp = opendir(~dir);
    if (dp == NULL) {
        return "ls: cannot access '" + dir + "': No such file or directory\n";
    }
    
    struct dirent* ep;
    String result;
    while ((ep = readdir(dp)) != NULL) {
        String name = ep->d_name;
        if (name != "." && name != "..") {
            if (ep->d_type == DT_DIR) {
                result += name + "/\n";
            } else {
                result += name + "\n";
            }
        }
    }
    closedir(dp);
    
    return result;
}

String InternalShell::CmdEcho(const Vector<String>& args) {
    String result;
    for(int i = 1; i < args.GetCount(); i++) {
        if (i > 1) result += " ";
        result += args[i];
    }
    result += "\n";
    return result;
}

String InternalShell::CmdHelp(const Vector<String>& args) {
    String help_text = 
        "Internal Shell Help:\n"
        "  cd [directory]  - Change directory\n"
        "  pwd             - Print working directory\n"
        "  ls [directory]  - List directory contents\n"
        "  echo [text]     - Print text\n"
        "  help            - Show this help\n"
        "  history         - Show command history\n"
        "  clear           - Clear screen\n";
    return help_text;
}

String InternalShell::CmdHistory(const Vector<String>& args) {
    String result;
    for(int i = 0; i < command_history.GetCount(); i++) {
        result += FormatInt(i + 1) + "  " + command_history[i] + "\n";
    }
    return result;
}

String InternalShell::CmdClear(const Vector<String>& args) {
    // Return an escape sequence to clear the terminal
    // This might need to be handled specially by the terminal
    return "\x1b[2J\x1b[H"; // ANSI escape codes for clear screen
}

Vector<String> InternalShell::ParseCommand(const String& cmd) {
    Vector<String> result;
    
    String current_token;
    bool in_quotes = false;
    char quote_char = 0;
    
    for(int i = 0; i < cmd.GetLength(); i++) {
        char c = cmd[i];
        
        if (!in_quotes && (c == '"' || c == '\'')) {
            in_quotes = true;
            quote_char = c;
        }
        else if (in_quotes && c == quote_char) {
            in_quotes = false;
        }
        else if (!in_quotes && (c == ' ' || c == '\t')) {
            if (!current_token.IsEmpty()) {
                result.Add(current_token);
                current_token.Clear();
            }
        }
        else {
            current_token.Cat(c);
        }
    }
    
    if (!current_token.IsEmpty()) {
        result.Add(current_token);
    }
    
    return result;
}

END_UPP_NAMESPACE