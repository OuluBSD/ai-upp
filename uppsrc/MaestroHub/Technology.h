#ifndef _MaestroHub_Technology_h_
#define _MaestroHub_Technology_h_

NAMESPACE_UPP

class TechnologyPane : public ParentCtrl {
public:
	Splitter split;
	RepoView repo;
	PlanView plan;
	
	Function<void(String track, String phase, String task)> WhenEnact;
	
	void Load(const String& root);

	typedef TechnologyPane CLASSNAME;
	TechnologyPane();
};

END_UPP_NAMESPACE

#endif
