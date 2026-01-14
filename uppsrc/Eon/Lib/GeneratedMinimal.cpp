#include "Lib.h"

// This file is generated. Do not modify this file.

NAMESPACE_UPP


#if (defined flagX11 && defined flagSCREEN)
String X11SwFboAtomSA::GetAction() {
	return "x11.sw.fbo.standalone";
}

AtomTypeCls X11SwFboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //X11SWFBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls X11SwFboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void X11SwFboAtomSA::Visit(Vis& v) {
	VIS_THIS(X11SwSinkDevice);
}

AtomTypeCls X11SwFboAtomSA::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagGUI && defined flagHAL && defined flagOGL && defined flagSCREEN)
String UppOglFboPipe::GetAction() {
	return "upp.fbo.sink";
}

AtomTypeCls UppOglFboPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //UPPOGLFBOPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(OGL,FBO),0);
	t.AddOut(VD(OGL,RECEIPT),0);
	return t;
}

LinkTypeCls UppOglFboPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void UppOglFboPipe::Visit(Vis& v) {
	VIS_THIS(UppOglScreenSinkDevice);
}

AtomTypeCls UppOglFboPipe::GetType() const {
	return GetAtomType();
}
#endif



END_UPP_NAMESPACE
