#ifndef SOFTAUDIO_INSTRUMENTJSON_H
#define SOFTAUDIO_INSTRUMENTJSON_H

#include "SoftAudio.h"
#include "InstrumentGraph.h"
#include <plugin/json/json.h>

NAMESPACE_AUDIO_BEGIN

// JSON serialization functions
void Jsonize(JsonIO& json, NoteDesc& x);
void Jsonize(JsonIO& json, VoiceConfig& x);
void Jsonize(JsonIO& json, InstrumentVoiceTemplate& x);
void Jsonize(JsonIO& json, InstrumentGraph& x);

NAMESPACE_AUDIO_END

#endif