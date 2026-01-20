#include "Maestro.h"

namespace Upp {

RepoView::RepoView() {
	Add(tree.SizePos());
}

void RepoView::Set(const Array<AssemblyInfo>& assemblies, const Array<PackageInfo>& packages) {
	tree.Clear();
	int root = tree.Add(0, CtrlImg::Dir(), "Repository");
	
	for(const auto& a : assemblies) {
		int aid = tree.Add(root, CtrlImg::Dir(), a.name);
		
		for(const auto& pkg_name : a.packages) {
			// Find package info
			const PackageInfo* pinfo = nullptr;
			for(const auto& p : packages) {
				if(p.name == pkg_name) {
					pinfo = &p;
					break;
				}
			}
			
			if(pinfo) {
				int pid = tree.Add(aid, CtrlImg::File(), pinfo->name + " [" + pinfo->build_system + "]");
				
				for(const auto& g : pinfo->groups) {
					int gid = tree.Add(pid, CtrlImg::Dir(), g.name);
					for(const auto& f : g.files) {
						tree.Add(gid, CtrlImg::File(), f);
					}
				}
				
				for(const auto& f : pinfo->ungrouped_files) {
					tree.Add(pid, CtrlImg::File(), f);
				}
			} else {
				tree.Add(aid, CtrlImg::File(), pkg_name + " (info missing)");
			}
		}
	}
	
	tree.Open(root);
}

}
