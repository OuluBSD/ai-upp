#include "Lib.h"

// This file is generated. Do not modify this file.

NAMESPACE_UPP


#if (defined flagFREEBSD && defined flagOPENHMD && defined flagVR) || (defined flagLINUX && defined flagOPENHMD && defined flagVR)
String OpenHMDPipe::GetAction() {
	return "x11.ogl.ohmd.events";
}

AtomTypeCls OpenHMDPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //OPENHMDPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls OpenHMDPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void OpenHMDPipe::Visit(Vis& v) {
	VIS_THIS(OpenHMDSinkDevice);
}

AtomTypeCls OpenHMDPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagWIN32 && defined flagOPENVR && defined flagVR)
String OpenVRPipe::GetAction() {
	return "openvr.ogl.holo.events";
}

AtomTypeCls OpenVRPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //OPENVRPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls OpenVRPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void OpenVRPipe::Visit(Vis& v) {
	VIS_THIS(OpenVRSinkDevice);
}

AtomTypeCls OpenVRPipe::GetType() const {
	return GetAtomType();
}
#endif



END_UPP_NAMESPACE
