#include "clicore.h"
#include "CoreGraph.h"

namespace Upp {

CoreGraph::CoreGraph() {
}

bool CoreGraph::BuildPackageGraph(const CoreWorkspace& ws, String& error) {
    // Clear existing graph data
    adj.Clear();
    rev.Clear();
    
    // Get all packages from workspace
    const Vector<String>& packages = ws.GetPackages();
    
    // Initialize adjacency lists for all packages
    for(int i = 0; i < packages.GetCount(); i++) {
        const String& pkg = packages[i];
        adj.GetAdd(pkg);
        rev.GetAdd(pkg);
    }
    
    // Build dependency graph from "uses" relationships
    for(int i = 0; i < packages.GetCount(); i++) {
        const String& pkg = packages[i];
        const Vector<String>& uses = ws.GetPackageUses(pkg);
        
        for(int j = 0; j < uses.GetCount(); j++) {
            const String& used_pkg = uses[j];
            
            // Validate that the used package exists
            if (!ws.HasPackage(used_pkg)) {
                error = "Package '" + pkg + "' uses non-existent package '" + used_pkg + "'";
                return false;
            }
            
            // Add dependency edge: pkg -> used_pkg
            adj.GetAdd(pkg).Add(used_pkg);
            // Add reverse edge: used_pkg <- pkg
            rev.GetAdd(used_pkg).Add(pkg);
        }
    }
    
    return true;
}

bool CoreGraph::TopologicalSort(Vector<String>& out_order, String& error) const {
    // Calculate in-degrees for all nodes
    Index<String, int> in_degree;
    
    // Initialize in-degree for all packages
    for(int i = 0; i < adj.GetCount(); i++) {
        const String& pkg = adj.GetKey(i);
        in_degree.GetAdd(pkg, 0);
    }
    
    // Calculate actual in-degrees
    for(int i = 0; i < adj.GetCount(); i++) {
        const String& pkg = adj.GetKey(i);
        const Vector<String>& deps = adj[i];
        for(int j = 0; j < deps.GetCount(); j++) {
            in_degree.GetAdd(deps[j])++;  // Increment in-degree of dependency
        }
    }
    
    // Find nodes with in-degree 0
    Vector<String> zero_indegree;
    for(int i = 0; i < in_degree.GetCount(); i++) {
        if(in_degree[i] == 0) {
            zero_indegree.Add(in_degree.GetKey(i));
        }
    }
    
    // Topological sort using Kahn's algorithm
    out_order.Clear();
    while(zero_indegree.GetCount() > 0) {
        String current = zero_indegree.Top();
        zero_indegree.Remove(zero_indegree.GetCount() - 1);  // Remove from end for efficiency
        out_order.Add(current);
        
        // Process all dependent packages of current
        const Vector<String>& deps = adj.Get(current, Vector<String>());
        for(int i = 0; i < deps.GetCount(); i++) {
            const String& dep = deps[i];
            in_degree.GetAdd(dep)--;  // Reduce in-degree
            if(in_degree.Get(dep) == 0) {
                zero_indegree.Add(dep);
            }
        }
    }
    
    // Check if all nodes were processed (no cycle)
    if(out_order.GetCount() != adj.GetCount()) {
        error = "Cycle detected in dependency graph";
        return false;
    }
    
    return true;
}

bool CoreGraph::DetectCycles(Vector<Vector<String>>& out_cycles) const {
    out_cycles.Clear();

    if(adj.GetCount() == 0) {
        return true;
    }

    // Use DFS to detect cycles
    Index<String, int> visited;  // 0 = unvisited, 1 = in progress, 2 = done
    Vector<String> path;

    for(int i = 0; i < adj.GetCount(); i++) {
        const String& pkg = adj.GetKey(i);
        if(visited.Get(pkg, 0) == 0) {
            if(DfsCycleDetection(pkg, adj, visited, path, out_cycles)) {
                // At least one cycle was found
                break;
            }
        }
    }

    return true;
}

// Helper function for cycle detection
static bool DfsCycleDetection(const String& node, const Index<String, Vector<String>>& adj,
                              Index<String, int>& visited,
                              Vector<String>& path, Vector<Vector<String>>& out_cycles) {
    visited.GetAdd(node, 1);  // Mark as in progress
    path.Add(node);

    // Check neighbors
    const Vector<String>& neighbors = adj.Get(node, Vector<String>());
    for(int i = 0; i < neighbors.GetCount(); i++) {
        const String& next = neighbors[i];
        if(visited.Get(next, 0) == 1) {
            // Found back edge - cycle detected
            Vector<String> cycle;
            int cycle_start = -1;
            for(int j = 0; j < path.GetCount(); j++) {
                if(path[j] == next) {
                    cycle_start = j;
                    break;
                }
            }
            if(cycle_start >= 0) {
                for(int j = cycle_start; j < path.GetCount(); j++) {
                    cycle.Add(path[j]);
                }
                cycle.Add(next); // Close the cycle
                out_cycles.Add(cycle);
            }
        } else if(visited.Get(next, 0) == 0) {
            if(DfsCycleDetection(next, adj, visited, path, out_cycles)) {
                return true; // Found a cycle
            }
        }
    }

    path.Remove(path.GetCount() - 1);  // Remove from path
    visited.GetAdd(node, 2);  // Mark as done
    return out_cycles.GetCount() > 0; // Return true if any cycles were found
}

bool CoreGraph::AffectedPackagesByFile(const String& filepath, const CoreWorkspace& ws, Vector<String>& out_packages) const {
    out_packages.Clear();

    // Determine which package owns the file
    String owner_pkg = ws.GetPackageOfFile(filepath);
    if(owner_pkg.IsEmpty()) {
        // File doesn't belong to any known package
        return false;
    }

    // Check if the owner package exists in our graph
    if(adj.Find(owner_pkg) < 0) {
        // Owner package not in dependency graph
        return false;
    }

    // Use BFS to find all packages that depend (directly or indirectly) on the owner package
    Vector<String> queue;
    Index<String, bool> visited;

    queue.Add(owner_pkg);
    visited.GetAdd(owner_pkg, true);
    out_packages.Add(owner_pkg); // Include the package that contains the file

    // Traverse the reverse graph to find all packages depending on the owner package
    while(queue.GetCount() > 0) {
        String current_pkg = queue.Top();
        queue.Remove(0);  // Remove from front

        // Get all packages that depend on current_pkg
        const Vector<String>& dependents = rev.Get(current_pkg, Vector<String>());
        for(int i = 0; i < dependents.GetCount(); i++) {
            const String& dependent_pkg = dependents[i];
            if(!visited.GetAdd(dependent_pkg, false)) {
                visited.GetAdd(dependent_pkg, true);
                out_packages.Add(dependent_pkg);
                queue.Add(dependent_pkg);
            }
        }
    }

    return true;
}

String CoreGraph::DumpGraph() const {
    String result;
    result << "Dependency Graph:\n";
    
    for(int i = 0; i < adj.GetCount(); i++) {
        const String& pkg = adj.GetKey(i);
        const Vector<String>& deps = adj[i];
        result << pkg << " -> ";
        for(int j = 0; j < deps.GetCount(); j++) {
            if(j > 0) result << ", ";
            result << deps[j];
        }
        result << "\n";
    }
    
    result << "\nReverse Dependencies:\n";
    for(int i = 0; i < rev.GetCount(); i++) {
        const String& pkg = rev.GetKey(i);
        const Vector<String>& dependents = rev[i];
        result << pkg << " <- ";
        for(int j = 0; j < dependents.GetCount(); j++) {
            if(j > 0) result << ", ";
            result << dependents[j];
        }
        result << "\n";
    }
    
    return result;
}

} // namespace Upp