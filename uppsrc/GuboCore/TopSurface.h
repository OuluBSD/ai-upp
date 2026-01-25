#ifndef _GuboCore_TopSurface_h_
#define _GuboCore_TopSurface_h_


NAMESPACE_UPP



class TopSurface :
        public Surface
{
	String title;

public:
        //RTTI_DECL1(TopSurface, Surface)
        TopSurface();
        virtual ~TopSurface() {}

	void Title(const String& s) { title = s; }
	String GetTitle() const { return title; }

        void CreateGeom2DComponent();	void UpdateFromTransform2D();
	void OpenMain();
	void Run();
	void RunInMachine();
	void FocusEvent();
	
	//Surface* GetSurface() override;
	
	
};


END_UPP_NAMESPACE


#endif
