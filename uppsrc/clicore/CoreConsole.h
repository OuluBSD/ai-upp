#ifndef _clicore_CoreConsole_h_
#define _clicore_CoreConsole_h_

#include <Core/Core.h>

using namespace Upp;

class CoreConsole {
public:
    CoreConsole();
    ~CoreConsole();

    // Console operations
    void Clear();
    void Append(const String& text);
    void AppendLine(const String& line);
    
    // Output retrieval
    String GetConsoleOutput() const;
    String GetErrorsOutput() const;
    
    // Console management
    void BeginGroup(const String& group);
    void EndGroup();
    void Flush();

private:
    String console_output;
    String error_output;
    Vector<String> groups;
    
    // Helper methods
    void PutConsole(const char* s);
    void PutErrorLine(const String& error);
};

#endif