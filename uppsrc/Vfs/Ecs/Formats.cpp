#include "Ecs.h"

NAMESPACE_UPP


void AudioFormat::Set(SoundSample::Type type, int channels, int freq, int sample_rate) {
	SampleBase<SoundSample>::SetType(type);
	DimBase<1>::SetSize(channels);
	TimeSeriesBase::SetTimeSeries(freq, sample_rate);
}

String AudioFormat::ToString() const {
	return		SampleBase<SoundSample>::ToString() + ", " +
				DimBase<1>::ToString() + ", " +
				TimeSeriesBase::ToString();
}

int AudioFormat::GetFrameSize() const {
	return		DimBase<1>::GetScalar() *
				TimeSeriesBase::GetSampleRate() *
				SampleBase<SoundSample>::GetSampleSize();
}

bool AudioFormat::IsValid() const {
	return		TimeSeriesBase::IsValid() && \
				SampleBase<SoundSample>::IsValid() && \
				DimBase<1>::IsValid();
}

bool AudioFormat::IsSame(const AudioFormat& b) const {
	return		TimeSeriesBase::IsSame(b) &&
				SampleBase<SoundSample>::IsCopyCompatible(b) &&
				DimBase<1>::IsSame(b);
}








int VideoFormat::default_width = 1280;
int VideoFormat::default_height = 720;

void VideoFormat::Set(ColorSampleFD::Type type, int w, int h, int freq, int sample_rate) {
	SampleBase<ColorSampleFD>::SetType(type);
	DimBase<2>::operator=(Size(w,h));
	TimeSeriesBase::SetTimeSeries(freq, sample_rate);
	cubemap = false;
}

int VideoFormat::GetFrameSize() const {
	return		SampleBase<ColorSampleFD>::GetSampleSize() *
				DimBase<2>::GetScalar() *
				TimeSeriesBase::GetSampleRate();
}

String VideoFormat::ToString() const {
	return		SampleBase<ColorSampleFD>::ToString() + ", " +
				DimBase<2>::ToString() + ", " +
				TimeSeriesBase::ToString();
}

bool VideoFormat::IsValid() const {
	return		TimeSeriesBase::IsValid() && \
				SampleBase<ColorSampleFD>::IsValid() && \
				DimBase<2>::IsValid();
}

bool VideoFormat::IsSame(const VideoFormat& b) const {
	return		TimeSeriesBase::IsSame(b) &&
				SampleBase<ColorSampleFD>::IsCopyCompatible(b) &&
				DimBase<2 >::IsSame(b);
}




void VolumeFormat::Set(BinarySample::Type type, int w, int h, int d, int freq, int sample_rate) {
	SampleBase<BinarySample>::SetType(type);
	DimBase<3>::operator=(Size3(w,h,d));
	TimeSeriesBase::SetTimeSeries(freq, sample_rate);
}

int VolumeFormat::GetFrameSize() const {
	return		SampleBase<BinarySample>::GetSampleSize() *
				DimBase<3>::GetVolume() *
				TimeSeriesBase::GetSampleRate();
}

String VolumeFormat::ToString() const {
	return		SampleBase<BinarySample>::ToString() + ", " +
				DimBase<3>::ToString() + ", " +
				TimeSeriesBase::ToString();
}

bool VolumeFormat::IsValid() const {
	return		TimeSeriesBase::IsValid() && \
				SampleBase<BinarySample>::IsValid() && \
				DimBase<3>::IsValid();
}

bool VolumeFormat::IsSame(const VolumeFormat& b) const {
	return		TimeSeriesBase::IsSame(b) &&
				SampleBase<BinarySample>::IsCopyCompatible(b) &&
				DimBase<3>::IsSame(b);
}

bool VolumeFormat::IsCopyCompatible(const VolumeFormat& b) const {
	return		SampleBase<BinarySample>::IsCopyCompatible(b);
}





void FboFormat::Set(BinarySample::Type t, int w, int h, int d, int freq, int sample_rate) {
	SampleBase<BinarySample>::SetType(t);
	DimBase<3>::operator=(Size3(w,h,d));
	TimeSeriesBase::SetTimeSeries(freq, sample_rate);
}

int FboFormat::GetFrameSize() const {
	return		SampleBase<BinarySample>::GetSampleSize() *
				DimBase<3>::GetScalar() *
				TimeSeriesBase::GetSampleRate();
}

String FboFormat::ToString() const {
	return		SampleBase<BinarySample>::ToString() + ", " +
				DimBase<3>::ToString() + ", " +
				TimeSeriesBase::ToString();
}

bool FboFormat::IsValid() const {
	return		TimeSeriesBase::IsValid() && \
				SampleBase<BinarySample>::IsValid() && \
				DimBase<3>::IsValid2D();
}

bool FboFormat::IsSame(const FboFormat& b) const {
	return		TimeSeriesBase::IsSame(b) &&
				SampleBase<BinarySample>::IsCopyCompatible(b) &&
				DimBase<3>::IsSame(b);
}




int MidiFormat::GetFrameSize() const {
	return		SampleBase<MidiSample>::GetSampleSize() *
				DimBase<1>::GetScalar() *
				SparseTimeSeriesBase::GetSampleRate();
}

String MidiFormat::ToString() const {
	return		SampleBase<MidiSample>::ToString() + ", " +
				DimBase<1>::ToString() + ", " +
				SparseTimeSeriesBase::ToString();
}

bool MidiFormat::IsValid() const {
	return true;
}

bool MidiFormat::IsSame(const MidiFormat& fmt) const {
	return true;
}







String EventFormat::ToString() const {
	return		SampleBase<EventSample>::ToString() + ", " +
				DimBase<1>::ToString() + ", " +
				SparseTimeSeriesBase::ToString();
}

bool EventFormat::IsValid() const {
	return true;
}

bool EventFormat::IsSame(const EventFormat& fmt) const {
	return true;
}

int EventFormat::GetFrameSize() const {
	return		DimBase<1>::GetScalar() *
				SparseTimeSeriesBase::GetSampleRate() *
				SampleBase<EventSample>::GetSampleSize();
}




String GuiFormat::ToString() const {
	return		SampleBase<GuiSample>::ToString() + ", " +
				DimBase<0>::ToString() + ", " +
				SparseTimeSeriesBase::ToString();
}

bool GuiFormat::IsValid() const {
	return true;
}

bool GuiFormat::IsSame(const GuiFormat& fmt) const {
	return true;
}

int GuiFormat::GetFrameSize() const {
	return		DimBase<0>::GetScalar() *
				SparseTimeSeriesBase::GetSampleRate() *
				SampleBase<GuiSample>::GetSampleSize() *
				text_block_size;
}




String ProgFormat::ToString() const {
	return		DimBase<1>::ToString();
}

bool ProgFormat::IsValid() const {
	return true;
}

bool ProgFormat::IsSame(const ProgFormat& fmt) const {
	return true;
}

int ProgFormat::GetFrameSize() const {
	return		DimBase<1>::GetScalar() *
				SparseTimeSeriesBase::GetSampleRate() *
				SampleBase<BinarySample>::GetSampleSize();
}








#define PROXY(x) \
	if (IsAudio()) return aud.x(); \
	if (IsVideo()) return vid.x(); \
	if (IsVolume()) return vol.x(); \
	if (IsMidi())  return mid.x(); \
	if (IsEvent()) return ev.x(); \
	if (IsFbo())   return fbo.x(); \
	if (IsProg())  return prog.x(); \
	Panic("Invalid type");

#define PROXY_(x,y) \
	if (IsAudio()) return aud.x((const AudioFormat&)y); \
	if (IsVideo()) return vid.x((const VideoFormat&)y); \
	if (IsVolume()) return vol.x((const VolumeFormat&)y); \
	if (IsMidi())  return mid.x((const MidiFormat&)y); \
	if (IsEvent()) return ev.x((const EventFormat&)y); \
	if (IsFbo())   return fbo.x((const FboFormat&)y); \
	if (IsProg())  return prog.x((const ProgFormat&)y); \
	Panic("Invalid type");

#define PROXY_CHK(x) ASSERT(IsValid()); PROXY(x)

#define PROXY_CHK_(x,y) ASSERT(IsValid()); PROXY_(x,y)

String ValueFormat::ToString() const {
	if (IsAudio()) return "AudioFormat(" + vd.ToString() + ", " + aud.ToString() + ")";
	if (IsVideo()) return "VideoFormat(" + vd.ToString() + ", " + vid.ToString() + ")";
	if (IsVolume()) return "VolumeFormat(" + vd.ToString() + ", " + vol.ToString() + ")";
	if (IsMidi())  return "MidiFormat(" + vd.ToString() + ", " + mid.ToString() + ")";
	if (IsEvent()) return "EventFormat(" + vd.ToString() + ", " + ev.ToString() + ")";
	if (IsFbo())   return "FboFormat(" + vd.ToString() + ", " + fbo.ToString() + ")";
	if (IsProg())   return "ProgFormat(" + vd.ToString() + ", " + prog.ToString() + ")";
	if (IsGui())   return "GuiFormat(" + vd.ToString() + ", " + gui.ToString() + ")";
	if (vd.val == ValCls::ORDER) return "OrderFormat";
	if (vd.val == ValCls::RECEIPT) return "ReceiptFormat";
	return "Invalid ValueFormat";
}

int ValueFormat::GetSampleSize() const {
	PROXY_CHK(GetSampleSize)
	return 0;
}

int ValueFormat::GetScalar() const {
	PROXY_CHK(GetScalar)
	return 0;
}

int ValueFormat::GetFrameSize() const {
	PROXY_CHK(GetFrameSize)
	return 0;
}

double ValueFormat::GetFrameSeconds() const {
	PROXY_CHK(GetFrameSeconds)
	return 0;
}

bool ValueFormat::HasData() const {
	return	vd.val != ValCls::ORDER &&
			vd.val != ValCls::RECEIPT;
}

bool ValueFormat::IsValid() const {
	if (!vd.IsValid()) return false;
	if (!HasData()) return true;
	PROXY(IsValid)
	return 0;
}

bool ValueFormat::IsSame(const ValueFormat& f) const {
	if (vd != f.vd) return false;
	if (!HasData()) return true;
	PROXY_CHK_(IsSame, f)
	return 0;
}

bool ValueFormat::IsCopyCompatible(const ValueFormat& f) const {
	if (vd != f.vd) return false;
	if (!HasData()) return true;
	PROXY_CHK_(IsCopyCompatible, f)
	return 0;
}

bool ValueFormat::operator ==(const ValueFormat& f) {
	return IsSame(f);
}

bool ValueFormat::operator !=(const ValueFormat& f) {
	return !IsSame(f);
}

void ValueFormat::SetDefault(ValDevCls t) {
	TODO
}

void ValueFormat::Clear() {
	vd.Clear();
	memset(data, 0, sizeof(data));
}


void ValueFormat::SetAudio(DevCls dev, SoundSample::Type t, int channels, int freq, int sample_rate) {
	vd.dev = dev;
	vd.val = ValCls::AUDIO;
	memset(data, 0, sizeof(data));
	aud.Set(t, channels, freq, sample_rate);
}

void ValueFormat::SetOrder(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::ORDER;
	memset(data, 0, sizeof(data));
}

void ValueFormat::SetReceipt(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::RECEIPT;
	memset(data, 0, sizeof(data));
}

void ValueFormat::SetMidi(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::MIDI;
	memset(data, 0, sizeof(data));
	mid.SetDefault();
}

void ValueFormat::SetVolume(DevCls dev, BinarySample::Type t, int w, int h, int d, int freq, int sample_rate) {
	vd.dev = dev;
	vd.val = ValCls::VOLUME;
	memset(data, 0, sizeof(data));
	vol.Set(t, w, h, d, freq, sample_rate);
	ASSERT(IsValid());
}

void ValueFormat::SetVideo(DevCls dev, ColorSampleFD::Type t, int w, int h, int freq, int sample_rate) {
	vd.dev = dev;
	vd.val = ValCls::VIDEO;
	memset(data, 0, sizeof(data));
	vid.Set(t, w, h, freq, sample_rate);
}

void ValueFormat::SetVideo(DevCls dev, const VideoFormat& vid) {
	vd.dev = dev;
	vd.val = ValCls::VIDEO;
	memset(data, 0, sizeof(data));
	this->vid = vid;
}

void ValueFormat::SetFbo(DevCls dev, BinarySample::Type t, int w, int h, int d, int freq, int sample_rate) {
	vd.dev = dev;
	vd.val = ValCls::FBO;
	memset(data, 0, sizeof(data));
	fbo.Set(t, w, h, d, freq, sample_rate);
	ASSERT(IsValid());
}

void ValueFormat::SetEvent(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::EVENT;
	memset(data, 0, sizeof(data));
}

void ValueFormat::SetProg(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::PROG;
	memset(data, 0, sizeof(data));
}

void ValueFormat::SetGui(DevCls dev) {
	vd.dev = dev;
	vd.val = ValCls::GUI;
	memset(data, 0, sizeof(data));
}

void ValueFormat::operator=(const ValueFormat& f) {
	vd = f.vd;
	memcpy(data, f.data, sizeof(data));
}



GVar::Sample GetGVarSampleFromBinarySample(BinarySample::Type t) {
	int sz = BinarySample::GetPackedSingleSize(t);
	if (BinarySample::IsFloating(t)) {
		if (sz == 4)
			return GVar::SAMPLE_FLOAT;
	}
	else {
		switch (sz) {
			case 1:	return GVar::SAMPLE_U8;
			case 2:	return GVar::SAMPLE_U16;
			case 4:	return BinarySample::IsSigned(t) ? GVar::SAMPLE_S32 : GVar::SAMPLE_U32;
			default: break;
		}
	}
	Panic("GetGvarSampleFromBinarySample: conversion failed");
	NEVER();
	return GVar::SAMPLE_FLOAT;
}




ValueFormat GetDefaultFormat(ValDevCls type) {
	//DUMP(type)
	ValueFormat fmt;
	
	if (type.val == ValCls::AUDIO) {
		fmt.SetAudio(type.dev, SoundSample::FLT_LE, 2, 44100, 128);
	}
	else if (type.val == ValCls::ORDER) {
		fmt.SetOrder(type.dev);
	}
	else if (type.val == ValCls::RECEIPT) {
		fmt.SetReceipt(type.dev);
	}
	else if (type.val == ValCls::MIDI) {
		fmt.SetMidi(type.dev);
	}
	else if (type.val == ValCls::VOLUME) {
		fmt.SetVolume(type.dev, BinarySample::U8_LE_A, 512, 512, 512, 1, 1);
	}
	else if (type.val == ValCls::VIDEO) {
		fmt.SetVideo(type.dev, ColorSampleFD::U8_LE_ABC, VideoFormat::default_width, VideoFormat::default_height, 60, 1);
	}
	else if (type.val == ValCls::EVENT) {
		fmt.SetEvent(type.dev);
	}
	else if (type.val == ValCls::FBO) {
		fmt.SetFbo(type.dev, BinarySample::U8_LE_ABC, VideoFormat::default_width, VideoFormat::default_height, 0, 60, 1);
	}
	else if (type.val == ValCls::PROG) {
		fmt.SetProg(type.dev);
	}
	else if (type.val == ValCls::GUI) {
		fmt.SetGui(type.dev);
	}
	else {
		TODO
	}
	
	return fmt;
}

static bool LoadIntField(const Value& v, int& out) {
	if (v.Is<int>()) {
		out = v.Get<int>();
		return true;
	}
	if (v.Is<int64>()) {
		out = int(v.Get<int64>());
		return true;
	}
	if (v.Is<double>()) {
		out = int(v.Get<double>());
		return true;
	}
	return false;
}

ValueMap StoreValDevTuple(const ValDevTuple& tuple) {
	ValueMap result;
	ValueArray channels;
	for (int i = 0; i < tuple.GetCount(); i++) {
		const ValDevTuple::Channel& ch = tuple[i];
		ValueMap entry;
		entry.Add("dev", int(ch.vd.dev));
		entry.Add("val", int(ch.vd.val));
		entry.Add("optional", ch.is_opt);
		channels.Add(entry);
	}
	if (channels.GetCount())
		result.Add("channels", channels);
	return result;
}

bool LoadValDevTuple(const Value& value, ValDevTuple& out) {
	if (IsNull(value)) {
		out.Clear();
		return true;
	}
	if (!value.Is<ValueMap>())
		return false;
	ValueMap map = value;
	Value channels_value = RouterLookupValue(map, "channels");
	if (IsNull(channels_value)) {
		out.Clear();
		return true;
	}
	if (!channels_value.Is<ValueArray>())
		return false;
	ValueArray channels = channels_value;
	out.Clear();
	for (int i = 0; i < channels.GetCount(); i++) {
		const Value& entry_value = channels[i];
		if (!entry_value.Is<ValueMap>())
			return false;
		ValueMap entry = entry_value;
		Value dev_value = RouterLookupValue(entry, "dev");
		Value val_value = RouterLookupValue(entry, "val");
		Value opt_value = RouterLookupValue(entry, "optional");
		int dev = 0;
		int val = 0;
		if (!LoadIntField(dev_value, dev) || !LoadIntField(val_value, val))
			return false;
		bool opt = opt_value.Is<bool>() ? opt_value.Get<bool>() : false;
		out.Add(ValDevCls(DevCls(dev), ValCls(val)), opt);
	}
	return true;
}

ValueMap StoreRouterPortDesc(const RouterPortDesc& desc) {
	ValueMap map;
	map.Add("direction", desc.direction == RouterPortDesc::Direction::Source ? "source" : "sink");
	map.Add("index", desc.index);
	if (!IsNull(desc.name))
		map.Add("name", desc.name);
	if (desc.vd.IsValid())
		map.Add("vd", StoreValDevTuple(desc.vd));
	if (!desc.metadata.IsEmpty())
		map.Add("metadata", Value(desc.metadata));
	return map;
}

static bool LoadRouterDirection(const Value& value, RouterPortDesc::Direction& dir) {
	if (value.Is<String>()) {
		String s = ToLower((String)value);
		if (s.StartsWith("source") || s.StartsWith("src"))
			dir = RouterPortDesc::Direction::Source;
		else if (s.StartsWith("sink"))
			dir = RouterPortDesc::Direction::Sink;
		else
			return false;
		return true;
	}
	int v = 0;
	if (!LoadIntField(value, v))
		return false;
	dir = v ? RouterPortDesc::Direction::Source : RouterPortDesc::Direction::Sink;
	return true;
}

bool LoadRouterPortDesc(const Value& value, RouterPortDesc& out) {
	if (!value.Is<ValueMap>())
		return false;
	ValueMap map = value;
	Value dir_value = RouterLookupValue(map, "direction");
	Value idx_value = RouterLookupValue(map, "index");
	if (IsNull(dir_value) || IsNull(idx_value))
		return false;
	if (!LoadRouterDirection(dir_value, out.direction))
		return false;
	if (!LoadIntField(idx_value, out.index))
		return false;
	Value name_value = RouterLookupValue(map, "name");
	if (name_value.Is<String>())
		out.name = name_value;
	Value vd_value = RouterLookupValue(map, "vd");
	if (!IsNull(vd_value) && !LoadValDevTuple(vd_value, out.vd))
		return false;
	Value meta_value = RouterLookupValue(map, "metadata");
	if (meta_value.Is<ValueMap>())
		out.metadata = meta_value;
	else
		out.metadata.Clear();
	return true;
}

ValueMap StoreRouterConnectionDesc(const RouterConnectionDesc& desc) {
	ValueMap map;
	map.Add("from_atom", desc.from_atom);
	map.Add("from_port", desc.from_port);
	map.Add("to_atom", desc.to_atom);
	map.Add("to_port", desc.to_port);
	if (!desc.metadata.IsEmpty())
		map.Add("metadata", Value(desc.metadata));
	return map;
}

bool LoadRouterConnectionDesc(const Value& value, RouterConnectionDesc& out) {
	if (!value.Is<ValueMap>())
		return false;
	ValueMap map = value;
	Value from_atom = RouterLookupValue(map, "from_atom");
	Value to_atom = RouterLookupValue(map, "to_atom");
	Value from_port = RouterLookupValue(map, "from_port");
	Value to_port = RouterLookupValue(map, "to_port");
	if (!from_atom.Is<String>() || !to_atom.Is<String>())
		return false;
	int from_idx = 0;
	int to_idx = 0;
	if (!LoadIntField(from_port, from_idx) || !LoadIntField(to_port, to_idx))
		return false;
	out.from_atom = from_atom;
	out.to_atom = to_atom;
	out.from_port = from_idx;
	out.to_port = to_idx;
	Value meta_value = RouterLookupValue(map, "metadata");
	if (meta_value.Is<ValueMap>())
		out.metadata = meta_value;
	else
		out.metadata.Clear();
	return true;
}

END_UPP_NAMESPACE
