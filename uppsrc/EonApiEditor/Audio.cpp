#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddAudio() {
	Package("Audio", "Aud");
	SetColor(0, 128, 0);
	Dependency("Eon");
	Dependency("Sound");
	Library("portaudio", "PORTAUDIO");
	HaveNegotiateFormat();
	
	Interface("SinkDevice", "AUDIO");
	Interface("SourceDevice", "AUDIO");
	
	Vendor("Portaudio", "BUILTIN_PORTAUDIO|PORTAUDIO");
	
}


END_UPP_NAMESPACE
