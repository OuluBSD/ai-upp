#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddAudioFileOut() {
	Package("AudioFileOut", "AFO");
	SetColor(0, 128, 0);
	HaveRecvFinalize();
	HaveIsReady();
	
	Interface("Sink", "AUDIO");
	
	Vendor("CoreAudio");
	
}


END_UPP_NAMESPACE
