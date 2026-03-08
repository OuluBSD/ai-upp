#ifndef _GuboCore_TopGubo_h_
#define _GuboCore_TopGubo_h_


NAMESPACE_UPP


class TopGubo :
        public Gubo
{
	String title;

public:
        //RTTI_DECL1(TopGubo, Gubo)
        TopGubo();
        virtual ~TopGubo() {}

	void Title(const String& s) { title = s; }
	String GetTitle() const { return title; }

        void CreateGeom3DComponent();	void UpdateFromTransform3D();
	void FocusEvent();
	void RunInMachine();
	void OpenMain();
	int Run();
	
};


END_UPP_NAMESPACE


#endif
