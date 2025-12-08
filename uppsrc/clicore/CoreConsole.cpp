#include "clicore.h"

using namespace Upp;

CoreConsole::CoreConsole() {
    // Initialize console
}

CoreConsole::~CoreConsole() {
    // Cleanup
}

void CoreConsole::Clear() {
    console_output.Clear();
    error_output.Clear();
}

void CoreConsole::Append(const String& text) {
    console_output.Cat(text);
}

void CoreConsole::AppendLine(const String& line) {
    console_output.Cat(line + "\n");
}

String CoreConsole::GetConsoleOutput() const {
    return console_output;
}

String CoreConsole::GetErrorsOutput() const {
    return error_output;
}

void CoreConsole::BeginGroup(const String& group) {
    groups.Add(group);
    AppendLine("Begin group: " + group);
}

void CoreConsole::EndGroup() {
    if(groups.GetCount() > 0) {
        String group = groups.Top();
        groups.Remove(groups.GetCount()-1);
        AppendLine("End group: " + group);
    }
}

void CoreConsole::Flush() {
    // In a CLI context, we might just ensure all output is written
    // For now, this is a no-op
}

void CoreConsole::PutConsole(const char* s) {
    Append(s);
}

void CoreConsole::PutErrorLine(const String& error) {
    error_output.Cat(error + "\n");
}