#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddEffect() {
	Package("Effect", "Fx");
	SetColor(0, 128, 0);
	Dependency("SoftAudio", "SOFTAUDIO");
	Dependency("api/AudioHost");
	Dependency("plugin/lilv", "LV2");
	HaveRecvFinalize();
	HaveIsReady();
	
	Interface("Effect", "AUDIO");
	
	Vendor("AudioCore", "SOFTAUDIO");
	Vendor("LV2", "LV2");
	
}


END_UPP_NAMESPACE
