#ifndef _AI_Ctrl_VideoSourceFile_h_
#define _AI_Ctrl_VideoSourceFile_h_

NAMESPACE_UPP

class VideoSourceFileCtrl : public WithVideoSourceFile<ComponentCtrl> {
	
public:
	typedef VideoSourceFileCtrl CLASSNAME;
	VideoSourceFileCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(VideoSourceFileCtrl)

class VideoSourceFileRangeCtrl : public WithVideoSourceFileRange<ComponentCtrl> {
	
public:
	typedef VideoSourceFileRangeCtrl CLASSNAME;
	VideoSourceFileRangeCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(VideoSourceFileRangeCtrl)

class VideoSourceFileAudioCtrl : public WithVideoSourceFileAudio<ComponentCtrl> {
	
public:
	typedef VideoSourceFileAudioCtrl CLASSNAME;
	VideoSourceFileAudioCtrl();
	
	void Data() override;
	void ToolMenu(Bar& bar) override;
	
	
};

INITIALIZE(VideoSourceFileAudioCtrl)

END_UPP_NAMESPACE

#endif
