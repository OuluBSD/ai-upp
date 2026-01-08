#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddSynth() {
	Package("Synth", "Syn");
	SetColor(0, 128, 0);
	Dependency("SoftInstru");
	Dependency("SoftSynth");
	Dependency("SoftAudio");
	Dependency("api/AudioHost");
	Dependency("plugin/fluidlite", "FLUIDLITE");
	Dependency("plugin/lilv", "LV2");
	Library("fluidsynth", "FLUIDSYNTH");
	HaveRecvFinalize();
	HaveIsReady();
	
	Interface("Instrument", "AUDIO&MIDI");
	
	Vendor("Soft");
	Vendor("Fluidsynth", "FLUIDSYNTH|FLUIDLITE");
	Vendor("FmSynth");
	Vendor("CoreSynth");
	Vendor("CoreDrummer");
	Vendor("LV2", "LV2");
	
}


END_UPP_NAMESPACE
