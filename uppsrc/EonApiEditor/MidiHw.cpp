#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddMidiHw() {
	Package("MidiHw", "Mid");
	SetColor(0, 128, 0);
	Dependency("MidiFile");
	Dependency("plugin/portmidi", "BUILTIN_PORTMIDI");
	Library("portmidi", "PORTMIDI");
	HaveIsReady();
	
	Interface("Source", "MIDI");
	
	Vendor("Portmidi", "PORTMIDI|BUILTIN_PORTMIDI");
	
}


END_UPP_NAMESPACE
