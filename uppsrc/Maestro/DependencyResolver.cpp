#include "Maestro.h"

namespace Upp {

Array<PackageInfo> DependencyResolver::TopologicalSort(const Array<PackageInfo>& packages) {
	VectorMap<String, Index<String>> graph;
	Index<String> all_nodes;
	VectorMap<String, int> in_degree;
	
	for(const auto& pkg : packages) {
		all_nodes.Add(pkg.name);
		Index<String>& deps = graph.GetAdd(pkg.name);
		for(const auto& dep : pkg.dependencies) {
			deps.Add(dep);
		}
		in_degree.GetAdd(pkg.name, 0);
	}
	
	for(const auto& pkg : packages) {
		for(const auto& dep : pkg.dependencies) {
			if(all_nodes.Find(dep) >= 0) {
				in_degree.GetAdd(dep)++;
			}
		}
	}
	
	Vector<String> queue;
	for(int i = 0; i < in_degree.GetCount(); i++) {
		if(in_degree[i] == 0)
			queue.Add(in_degree.GetKey(i));
	}
	
	Array<PackageInfo> result;
	Index<String> processed;
	
	while(!queue.IsEmpty()) {
		String current_name = queue[0];
		queue.Remove(0);
		
		for(const auto& pkg : packages) {
			if(pkg.name == current_name) {
				result.Add(pkg);
				processed.Add(current_name);
				break;
			}
		}
		
		for(int i = 0; i < graph.GetCount(); i++) {
			if(graph[i].Find(current_name) >= 0) {
				String target_name = graph.GetKey(i);
				int& deg = in_degree.Get(target_name);
				deg--;
				if(deg == 0)
					queue.Add(target_name);
			}
		}
	}
	
	if(result.GetCount() != all_nodes.GetCount()) {
		throw Exc("Circular dependency detected in package graph.");
	}
	
	return result;
}

Vector<String> DependencyResolver::GetDependencies(const Array<PackageInfo>& all_packages, const String& start_package) {
	Index<String> dependencies;
	Index<String> visited;
	
	VectorMap<String, const PackageInfo*> name_to_pkg;
	for(const auto& p : all_packages) name_to_pkg.Add(p.name, &p);
	
	Vector<String> stack;
	stack.Add(start_package);
	
	while(!stack.IsEmpty()) {
		String curr = stack.Pop();
		if(visited.Find(curr) >= 0) continue;
		visited.Add(curr);
		
		int idx = name_to_pkg.Find(curr);
		if(idx >= 0) {
			const PackageInfo* p = name_to_pkg[idx];
			for(const auto& dep : p->dependencies) {
				dependencies.Add(dep);
				stack.Add(dep);
			}
		}
	}
	
	return dependencies.PickKeys();
}

Vector<String> DependencyResolver::GetDependents(const Array<PackageInfo>& all_packages, const String& start_package) {
	Index<String> dependents;
	Index<String> visited;
	
	VectorMap<String, Vector<String>> reverse_graph;
	for(const auto& p : all_packages) {
		for(const auto& dep : p.dependencies) {
			reverse_graph.GetAdd(dep).Add(p.name);
		}
	}
	
	Vector<String> stack;
	stack.Add(start_package);
	
	while(!stack.IsEmpty()) {
		String curr = stack.Pop();
		if(visited.Find(curr) >= 0) continue;
		visited.Add(curr);
		
		int idx = reverse_graph.Find(curr);
		if(idx >= 0) {
			for(const auto& dep_pkg : reverse_graph[idx]) {
				dependents.Add(dep_pkg);
				stack.Add(dep_pkg);
			}
		}
	}
	
	return dependents.PickKeys();
}

}