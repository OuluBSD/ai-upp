#ifndef _Maestro_PlanView_h_
#define _Maestro_PlanView_h_

class PlanView : public ParentCtrl {
public:
	TreeCtrl tree;
	
	void Set(const Array<Track>& tracks);

	typedef PlanView CLASSNAME;
	PlanView();
};

#endif