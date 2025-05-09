#ifndef _EcsLib_Abs2D_h_
#define _EcsLib_Abs2D_h_

#if 0

namespace Ecs {
	class Geom2DComponent;
}

class Absolute2D :
	public Absolute2DInterface
{
	
public:
	ATOM_CTOR_(Absolute2D, Absolute2DInterface)
	Absolute2D();
	virtual ~Absolute2D() {}
	
	void						Title(const String& title) override;
	Absolute2DInterface&		Sizeable(bool b=true) override;
	Absolute2DInterface&		MaximizeBox(bool b=true) override;
	Absolute2DInterface&		MinimizeBox(bool b=true) override;
	int							Run(bool appmodal=false) override;
	String						GetTitle() const override;
	
	
	Upp::Ecs::Geom2DComponent* GetWindow();
	Upp::Ecs::Windows* GetWindows();
	
	
protected:
	Upp::Ecs::Geom2DComponent* cw = NULL;
	int id;
	
	
public:
	void Init(Upp::Ecs::Geom2DComponent* cw, int id);
	
	Upp::Ecs::Geom2DComponent* GetWindow() const {return cw;}
	Upp::Ecs::WindowManager* GetWindowManager() const;
	
};


#endif
#endif
