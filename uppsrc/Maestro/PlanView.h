#ifndef _Maestro_PlanView_h_
#define _Maestro_PlanView_h_

class PlanView : public ParentCtrl {
public:
	TreeCtrl tree;
	
	Function<void(String track, String phase, String task)> WhenEnact;
	
	void Set(const Array<Track>& tracks);

	typedef PlanView CLASSNAME;
	PlanView();
};

#endif