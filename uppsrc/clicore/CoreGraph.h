#ifndef UPP_Clicore_COREGRAPH_H
#define UPP_Clicore_COREGRAPH_H

#include <Core/Core.h>

#include "CoreWorkspace.h"

namespace Upp {

class CoreGraph : Moveable<CoreGraph> {
public:
    CoreGraph();

    // Build package dependency graph
    bool BuildPackageGraph(const CoreWorkspace& ws, String& error);

    // Graph queries
    bool TopologicalSort(Vector<String>& out_order, String& error) const;
    bool DetectCycles(Vector<Vector<String>>& out_cycles) const;

    // Impact analysis - needs workspace to map files to packages
    bool AffectedPackagesByFile(const String& filepath,
                                const CoreWorkspace& ws,
                                Vector<String>& out_packages) const;

    // Debug dump
    String DumpGraph() const;

private:
    // package -> list of dependent packages (uses)
    VectorMap<String, Vector<String>> adj;

    // reverse edges (package -> who depends on it)
    VectorMap<String, Vector<String>> rev;

};

}

#endif