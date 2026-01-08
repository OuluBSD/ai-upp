#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddHolographic() {
	Package("Holograph", "Holo");
	SetColor(0, 128, 0);
	Dependency("api/Media");
	Dependency("plugin/hcidump", "LINUX");
	Dependency("SoftHMD", "SOFTHMD");
	Library("openhmd hidapi-libusb", "LINUX & OPENHMD");
	HaveRecvFinalize();
	HaveIsReady();
	
	Interface("SinkDevice", "VR");
	
	Vendor("OpenHMD", "LINUX&OPENHMD|FREEBSD&OPENHMD");
	Vendor("LocalHMD", "LOCALHMD");
	Vendor("RemoteVRServer", "LINUX|FREEBSD");
	Vendor("DevUsb", "LINUX|FREEBSD");
	Vendor("DevBluetooth", "LINUX&HACK|FREEBSD&HACK");
	Vendor("OpenVR", "WIN32&OPENVR");
	
	
}


END_UPP_NAMESPACE
