#ifndef _Maestro_RepoView_h_
#define _Maestro_RepoView_h_

class RepoView : public ParentCtrl {
public:
	TreeCtrl tree;
	
	void Set(const Array<AssemblyInfo>& assemblies, const Array<PackageInfo>& packages);

	typedef RepoView CLASSNAME;
	RepoView();
};

#endif