#ifndef _SoftPhys_Feedback_h_
#define _SoftPhys_Feedback_h_

NAMESPACE_UPP
namespace SoftPhys {


struct Feedback : Object {
	//RTTI_DECL1(Feedback, Object)
	using Object::Object;
	
	
	Feedback() {}
	
	void Visit(Vis& vis) override {VIS_THIS(Object);}
};


}
END_UPP_NAMESPACE

#endif
