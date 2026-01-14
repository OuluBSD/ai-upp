// This file have been generated automatically.
// DO NOT MODIFY THIS FILE!

#if (defined flagAUDIO && defined flagMIDI)
static bool Instrument_Create(NativeInstrument*& dev);
static void Instrument_Destroy(NativeInstrument*& dev);
static bool Instrument_Initialize(NativeInstrument&, AtomBase&, const WorldState&);
static bool Instrument_PostInitialize(NativeInstrument&, AtomBase&);
static bool Instrument_Start(NativeInstrument&, AtomBase&);
static void Instrument_Stop(NativeInstrument&, AtomBase&);
static void Instrument_Uninitialize(NativeInstrument&, AtomBase&);
static bool Instrument_Send(NativeInstrument&, AtomBase&, RealtimeSourceConfig& cfg, PacketValue& out, int src_ch);
static void Instrument_Visit(NativeInstrument&, AtomBase&, Visitor& vis);
static bool Instrument_Recv(NativeInstrument&, AtomBase&, int, const Packet&);
static void Instrument_Finalize(NativeInstrument&, AtomBase&, RealtimeSourceConfig&);
static bool Instrument_IsReady(NativeInstrument&, AtomBase&, PacketIO& io);
static bool Instrument_IsDebugSoundEnabled(const NativeInstrument&, const AtomBase&);
static String Instrument_GetDebugSoundOutput(const NativeInstrument&, const AtomBase&);
static int Instrument_GetDebugSoundSeed(const NativeInstrument&, const AtomBase&);
static bool Instrument_IsDebugPrintEnabled(const NativeInstrument&, const AtomBase&);
#endif

