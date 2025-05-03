#ifndef _EonDraw_Binder_h_
#define _EonDraw_Binder_h_


class Draw;

using namespace UPP;

#undef Bool
typedef unsigned char	Bool;

typedef float			Float;
//typedef float			Clamp;
typedef int				Int;
typedef unsigned int	Uint;
typedef unsigned char	Byte;
typedef void			Void;

typedef float			TimeSpan;

class EnvState;


class BinderIfaceEvents {
	
public:
	virtual ~BinderIfaceEvents() {}
	
	virtual void Dispatch(const CtrlEvent& state) = 0;
	
};


class BinderIfaceVideo {
	
public:
	BinderIfaceVideo();
	virtual ~BinderIfaceVideo() {}
	
	virtual bool Render(Draw& d) = 0;
	virtual bool Arg(const String& key, const String& value) = 0;
	
	virtual bool RenderProg(DrawCommand*& begin, DrawCommand*& end) {return false;}
	virtual void Initialize() {}
	virtual void Uninitialize() {}
	virtual void Visit(Vis& vis) {}
	
};


/*class CpuBuffer;
class CpuShader;

class BinderIfaceCpu {
	
public:
	
	virtual void Render(const CpuBuffer& buf, CpuShader& shader) = 0;
	
};


class OglBuffer;
class OglShader;

class BinderIfaceOgl {
	
public:
	
	virtual void Render(const OglBuffer& buf, OglShader& shader) = 0;
	
};*/



#endif
