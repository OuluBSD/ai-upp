#ifndef _EonLib_GeneratedMinimal_h_
#define _EonLib_GeneratedMinimal_h_

// This file is generated. Do not modify this file.

#if (defined flagX11 && defined flagSCREEN)
class X11SwFboAtomSA : public X11SwSinkDevice {

public:
	ATOM_CTOR_(X11SwFboAtomSA, X11SwSinkDevice)
	//ATOMTYPE(X11SwFboAtomSA)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#if (defined flagGUI && defined flagHAL && defined flagOGL)
class UppOglFboPipe : public UppOglVideoSinkDevice {

public:
	ATOM_CTOR_(UppOglFboPipe, UppOglVideoSinkDevice)
	//ATOMTYPE(UppOglFboPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#endif
