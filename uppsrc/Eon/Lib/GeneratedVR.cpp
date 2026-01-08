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


#if (defined flagLOCALHMD && defined flagVR)
String LocalHMDPipe::GetAction() {
	return "x11.ogl.holo.events";
}

AtomTypeCls LocalHMDPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //LOCALHMDPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls LocalHMDPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void LocalHMDPipe::Visit(Vis& v) {
	VIS_THIS(LocalHMDSinkDevice);
}

AtomTypeCls LocalHMDPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFREEBSD && defined flagVR) || (defined flagLINUX && defined flagVR)
String RemoteVRServerPipe::GetAction() {
	return "tcp.ogl.holo.events";
}

AtomTypeCls RemoteVRServerPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //REMOTEVRSERVERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls RemoteVRServerPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void RemoteVRServerPipe::Visit(Vis& v) {
	VIS_THIS(RemoteVRServerSinkDevice);
}

AtomTypeCls RemoteVRServerPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFREEBSD && defined flagHACK && defined flagVR) || (defined flagLINUX && defined flagHACK && defined flagVR)
String BluetoothHoloPipe::GetAction() {
	return "bluetooth.holo.events";
}

AtomTypeCls BluetoothHoloPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //BLUETOOTHHOLOPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls BluetoothHoloPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void BluetoothHoloPipe::Visit(Vis& v) {
	VIS_THIS(DevBluetoothSinkDevice);
}

AtomTypeCls BluetoothHoloPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagHAL)
String HoloContextAtom::GetAction() {
	return "holo.context";
}

AtomTypeCls HoloContextAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOCONTEXTATOM;
	t.role = AtomRole::DRIVER;
	t.AddIn(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls HoloContextAtom::GetLinkType() {
	return LINKTYPE(DRIVER, DRIVER);
}

void HoloContextAtom::Visit(Vis& v) {
	VIS_THIS(HoloContextBase);
}

AtomTypeCls HoloContextAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagHAL)
String HoloEventAtomPipe::GetAction() {
	return "holo.event.pipe";
}

AtomTypeCls HoloEventAtomPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOEVENTATOMPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,EVENT),0);
	return t;
}

LinkTypeCls HoloEventAtomPipe::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void HoloEventAtomPipe::Visit(Vis& v) {
	VIS_THIS(HoloEventsBase);
}

AtomTypeCls HoloEventAtomPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagUWP && defined flagDX12 && defined flagDX12)
String HoloD12FboAtomSA::GetAction() {
	return "holo.fbo.standalone";
}

AtomTypeCls HoloD12FboAtomSA::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //HOLOD12FBOATOMSA;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(DX,ORDER),0);
	t.AddOut(VD(DX,RECEIPT),0);
	return t;
}

LinkTypeCls HoloD12FboAtomSA::GetLinkType() {
	return LINKTYPE(POLLER_PIPE, PROCESS);
}

void HoloD12FboAtomSA::Visit(Vis& v) {
	VIS_THIS(HoloD12VideoSinkDevice);
}

AtomTypeCls HoloD12FboAtomSA::GetType() const {
	return GetAtomType();
}
#endif



END_UPP_NAMESPACE
