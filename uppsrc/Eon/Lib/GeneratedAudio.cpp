#include "Lib.h"

// This file is generated. Do not modify this file.

NAMESPACE_UPP


#if defined flagMIDI
String MidiFileReaderPipe::GetAction() {
	return "midi.file.reader.pipe";
}

AtomTypeCls MidiFileReaderPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,MIDI),0);
	return t;
}

LinkTypeCls MidiFileReaderPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void MidiFileReaderPipe::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReaderPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiFileReader::GetAction() {
	return "midi.file.reader";
}

AtomTypeCls MidiFileReader::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADER;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls MidiFileReader::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void MidiFileReader::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReader::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiFileReader16::GetAction() {
	return "midi.file.reader16";
}

AtomTypeCls MidiFileReader16::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDIFILEREADER16;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls MidiFileReader16::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void MidiFileReader16::Visit(Vis& v) {
	VIS_THIS(MidiFileReaderAtom);
}

AtomTypeCls MidiFileReader16::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagMIDI
String MidiNullSink::GetAction() {
	return "midi.null.sink";
}

AtomTypeCls MidiNullSink::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //MIDINULLSINK;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,MIDI),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls MidiNullSink::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void MidiNullSink::Visit(Vis& v) {
	VIS_THIS(MidiNullAtom);
}

AtomTypeCls MidiNullSink::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagFLUIDLITE && defined flagAUDIO && defined flagMIDI) || (defined flagFLUIDSYNTH && defined flagAUDIO && defined flagMIDI)
String FluidsynthPipe::GetAction() {
	return "fluidsynth.pipe";
}

AtomTypeCls FluidsynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //FLUIDSYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls FluidsynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void FluidsynthPipe::Visit(Vis& v) {
	VIS_THIS(FluidsynthInstrument);
}

AtomTypeCls FluidsynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagAUDIO && defined flagMIDI)
String SoftInstrumentPipe::GetAction() {
	return "softinstru.pipe";
}

AtomTypeCls SoftInstrumentPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //SOFTINSTRUMENTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls SoftInstrumentPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void SoftInstrumentPipe::Visit(Vis& v) {
	VIS_THIS(SoftInstrument);
}

AtomTypeCls SoftInstrumentPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagAUDIO && defined flagMIDI)
String FmSynthPipe::GetAction() {
	return "fmsynth.pipe";
}

AtomTypeCls FmSynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //FMSYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls FmSynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void FmSynthPipe::Visit(Vis& v) {
	VIS_THIS(FmSynthInstrument);
}

AtomTypeCls FmSynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagLV2 && defined flagAUDIO && defined flagMIDI)
String LV2InstrumentPipe::GetAction() {
	return "lv2.instrument.pipe";
}

AtomTypeCls LV2InstrumentPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //LV2INSTRUMENTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls LV2InstrumentPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void LV2InstrumentPipe::Visit(Vis& v) {
	VIS_THIS(LV2Instrument);
}

AtomTypeCls LV2InstrumentPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagAUDIO && defined flagMIDI)
String CoreSynthPipe::GetAction() {
	return "coresynth.pipe";
}

AtomTypeCls CoreSynthPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //CORESYNTHPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreSynthPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreSynthPipe::Visit(Vis& v) {
	VIS_THIS(CoreSynthInstrument);
}

AtomTypeCls CoreSynthPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagAUDIO && defined flagMIDI)
String CoreDrummerPipe::GetAction() {
	return "coredrummer.pipe";
}

AtomTypeCls CoreDrummerPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREDRUMMERPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),1);
	return t;
}

LinkTypeCls CoreDrummerPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreDrummerPipe::Visit(Vis& v) {
	VIS_THIS(CoreDrummerInstrument);
}

AtomTypeCls CoreDrummerPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagAUDIO
String CoreEffectPipe::GetAction() {
	return "corefx.pipe";
}

AtomTypeCls CoreEffectPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREEFFECTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreEffectPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CoreEffectPipe::Visit(Vis& v) {
	VIS_THIS(AudioCoreEffect);
}

AtomTypeCls CoreEffectPipe::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagAUDIO
String CoreEffectAtom::GetAction() {
	return "corefx.atom";
}

AtomTypeCls CoreEffectAtom::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREEFFECTATOM;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddIn(VD(CENTER,AUDIO),1);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls CoreEffectAtom::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void CoreEffectAtom::Visit(Vis& v) {
	VIS_THIS(AudioCoreEffect);
}

AtomTypeCls CoreEffectAtom::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagLV2 && defined flagAUDIO)
String LV2EffectPipe::GetAction() {
	return "lv2.effect.pipe";
}

AtomTypeCls LV2EffectPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //LV2EFFECTPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,AUDIO),0);
	return t;
}

LinkTypeCls LV2EffectPipe::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void LV2EffectPipe::Visit(Vis& v) {
	VIS_THIS(LV2Effect);
}

AtomTypeCls LV2EffectPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
String PortmidiPipe::GetAction() {
	return "midi.src.portmidi";
}

AtomTypeCls PortmidiPipe::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //PORTMIDIPIPE;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,MIDI),0);
	return t;
}

LinkTypeCls PortmidiPipe::GetLinkType() {
	return LINKTYPE(PIPE, PROCESS);
}

void PortmidiPipe::Visit(Vis& v) {
	VIS_THIS(PortmidiSource);
}

AtomTypeCls PortmidiPipe::GetType() const {
	return GetAtomType();
}
#endif


#if (defined flagBUILTIN_PORTMIDI && defined flagMIDI) || (defined flagPORTMIDI && defined flagMIDI)
String PortmidiSend::GetAction() {
	return "midi.src.side.portmidi";
}

AtomTypeCls PortmidiSend::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //PORTMIDISEND;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,ORDER),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	t.AddOut(VD(CENTER,MIDI),1);
	return t;
}

LinkTypeCls PortmidiSend::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void PortmidiSend::Visit(Vis& v) {
	VIS_THIS(PortmidiSource);
}

AtomTypeCls PortmidiSend::GetType() const {
	return GetAtomType();
}
#endif


#if defined flagAUDIO
String CoreAudioFileOut::GetAction() {
	return "audio.file.writer";
}

AtomTypeCls CoreAudioFileOut::GetAtomType() {
	AtomTypeCls t;
	t.sub = SUB_ATOM_CLS; //COREAUDIOFILEOUT;
	t.role = AtomRole::PIPE;
	t.AddIn(VD(CENTER,AUDIO),0);
	t.AddOut(VD(CENTER,RECEIPT),0);
	return t;
}

LinkTypeCls CoreAudioFileOut::GetLinkType() {
	return LINKTYPE(PIPE_OPTSIDE, PROCESS);
}

void CoreAudioFileOut::Visit(Vis& v) {
	VIS_THIS(CoreAudioSink);
}

AtomTypeCls CoreAudioFileOut::GetType() const {
	return GetAtomType();
}
#endif



END_UPP_NAMESPACE
