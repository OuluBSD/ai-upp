#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddVolumetric() {
	Package("Volumetric", "Vol");
	SetColor(0, 128, 0);
	HaveIsReady();
	
	Interface("StaticSource", "VOLUMETRIC");
	
	Vendor("RawByte");
	
}


END_UPP_NAMESPACE
