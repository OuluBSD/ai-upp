#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddCamera() {
	Package("Camera", "Cam");
	SetColor(0, 128, 0);
	Dependency("api/Media");
	HaveIsReady();
	
	Interface("Camera", "CAMERA");
	
	Vendor("V4L2OpenCV", "OPENCV&LINUX");
	
}


END_UPP_NAMESPACE
