#ifndef _clicore_CoreTelemetry_h_
#define _clicore_CoreTelemetry_h_

#include <Core/Core.h>
#include <clicore/CoreGraph.h>
#include <clicore/CoreWorkspace.h>

class CoreTelemetry : Moveable<CoreTelemetry> {
public:
    CoreTelemetry();

    // Structural analytics
    Value GetWorkspaceStats(const CoreWorkspace& ws) const;
    Value GetPackageStats(const CoreWorkspace::Package& pkg) const;

    // Code complexity heuristics
    Value ComputeFileComplexity(const String& path, const String& contents) const;

    // Dependency graph analytics
    Value GetGraphStats(const CoreGraph& graph) const;

    // Change tracking (lightweight for v1)
    void RecordEdit(const String& path, int bytes_before, int bytes_after);
    Value GetEditHistory() const;

private:
    struct EditRecord : Moveable<EditRecord> {
        String path;
        int delta;
        Time timestamp;
    };
    Vector<EditRecord> edits;
};

#endif