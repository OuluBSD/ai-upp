#ifndef _Draw_Video_VideoEffects_h_
#define _Draw_Video_VideoEffects_h_

class VideoEffect {
public:
	virtual ~VideoEffect() = default;
	virtual void Process(VideoFrame& frame) = 0;
};

#endif
