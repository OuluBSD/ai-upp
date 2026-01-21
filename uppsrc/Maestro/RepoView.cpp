#include "Maestro.h"

#ifdef flagGUI

NAMESPACE_UPP

RepoView::RepoView() {
	Add(tree.SizePos());
}

void RepoView::Set(const Array<AssemblyInfo>& assemblies, const Array<PackageInfo>& packages) {
	tree.Clear();
	int root_id = tree.Add(0, CtrlImg::Dir(), "Repository");
	
	for(const auto& a : assemblies) {
		int aid = tree.Add(root_id, CtrlImg::Dir(), a.name);
		for(const auto& p : a.packages) {
			tree.Add(aid, CtrlImg::File(), p);
		}
	}
	tree.OpenDeep(root_id);
}

END_UPP_NAMESPACE

#endif