#ifndef _IMedia_Audio_h_
#define _IMedia_Audio_h_

NAMESPACE_UPP


template <class Backend>
class AudioFrameT : public PacketBufferBase {
	
	
public:
	//RTTI_DECL1(AudioFrameT, PacketBufferBase)
	virtual ~AudioFrameT() {}
	
	
	ValueFormat fmt;
	
	
	const ValueFormat&		GetFormat() const {return fmt;}
};


template <class Backend>
class AudioInputFrameT : public AudioFrameT<Backend> {
	
public:
	using Base = AudioFrameT<Backend>;
	//RTTI_DECL1(AudioInputFrameT, Base)
	
	
};



















END_UPP_NAMESPACE

#endif
