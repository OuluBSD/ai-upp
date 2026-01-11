#ifndef _EonLib_GeneratedVR_h_
#define _EonLib_GeneratedVR_h_

// This file is generated. Do not modify this file.

#if (defined flagFREEBSD && defined flagOPENHMD && defined flagVR) || (defined flagLINUX && defined flagOPENHMD && defined flagVR)
class OpenHMDPipe : public OpenHMDSinkDevice {

public:
	ATOM_CTOR_(OpenHMDPipe, OpenHMDSinkDevice)
	//ATOMTYPE(OpenHMDPipe)
	static String GetAction();
	static AtomTypeCls GetAtomType();
	static LinkTypeCls GetLinkType();
	void Visit(Vis& v) override;
	AtomTypeCls GetType() const override;
	
};
#endif

#endif
