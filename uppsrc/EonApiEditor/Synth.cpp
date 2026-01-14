#include "EonApiEditor.h"

NAMESPACE_UPP


void InterfaceBuilder::AddSynth() {
	Package("Synth", "Syn");
	SetColor(0, 128, 0);
	Dependency("SoftInstru", "SOFTINSTRU");
	Dependency("SoftSynth", "SOFTSYNTH");
	Dependency("SoftAudio", "SOFTAUDIO");
	Dependency("api/AudioHost");
	Dependency("plugin/fluidlite", "FLUIDLITE");
	Dependency("plugin/lilv", "LV2");
	Library("fluidsynth", "FLUIDSYNTH");
	HaveRecvFinalize();
	HaveIsReady();
	HaveSoundFunctions();
	HaveDebugFunctions();
	
	Interface("Instrument", "AUDIO&MIDI");
	
	Vendor("Soft", "SOFTINSTRU");
	Vendor("Fluidsynth", "FLUIDSYNTH|FLUIDLITE");
	Vendor("FmSynth", "SOFTSYNTH");
	Vendor("CoreSynth", "SOFTAUDIO");
	Vendor("CoreDrummer", "SOFTAUDIO");
	Vendor("LV2", "LV2");
	
}


END_UPP_NAMESPACE
