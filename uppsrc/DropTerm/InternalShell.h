#ifndef _DropTerm_InternalShell_h_
#define _DropTerm_InternalShell_h_

#include <Core/Core.h>
#include <plugin/utf8/utf8.h>

NAMESPACE_UPP

class InternalShell {
private:
    String prompt;
    String current_directory;
    Vector<String> command_history;
    
public:
    InternalShell();
    ~InternalShell();
    
    void SetPrompt(const String& p) { prompt = p; }
    String GetPrompt() const { return prompt; }
    
    String ExecuteCommand(const String& cmd);
    String ProcessInput(const String& input);
    
    // Basic command implementations
    String CmdCd(const Vector<String>& args);
    String CmdPwd(const Vector<String>& args);
    String CmdLs(const Vector<String>& args);
    String CmdEcho(const Vector<String>& args);
    String CmdHelp(const Vector<String>& args);
    String CmdHistory(const Vector<String>& args);
    String CmdClear(const Vector<String>& args);
    
    // Utility functions
    Vector<String> ParseCommand(const String& cmd);
    String GetCurrentDirectory() const { return current_directory; }
    void SetCurrentDirectory(const String& dir) { current_directory = dir; }
};

END_UPP_NAMESPACE

#endif