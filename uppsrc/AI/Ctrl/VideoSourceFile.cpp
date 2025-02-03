#include "Ctrl.h"

NAMESPACE_UPP

VideoSourceFileCtrl::VideoSourceFileCtrl() {
	CtrlLayout(*this);
	
	this->browse.WhenAction = [this]{
		VideoSourceFile& comp = this->GetExt<VideoSourceFile>();
		ValueMap map = comp.value;
		String path = map("path");
		String dir = path.Is() ? GetFileDirectory(path) : GetHomeDirectory();
		FileSelNative sel;
		sel.ActiveDir(dir);
		if (path.Is())
			sel.Set(GetFileName(path));
		if (sel.ExecuteOpen("Select video file")) {
			map = comp.value;
			map.Set("path", sel.Get());
			comp.value = map;
			PostCallback(THISBACK(Data));
		}
	};
	
	
}

void VideoSourceFileCtrl::Data() {
	VideoSourceFile& comp = this->GetExt<VideoSourceFile>();
	ValueMap map = comp.value;
	this->path.SetData(map("path"));
}

void VideoSourceFileCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(VideoSourceFile, VideoSourceFileCtrl)






VideoSourceFileRangeCtrl::VideoSourceFileRangeCtrl() {
	CtrlLayout(*this);
	
}

void VideoSourceFileRangeCtrl::Data() {
	
}

void VideoSourceFileRangeCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(VideoSourceFileRange, VideoSourceFileRangeCtrl)






VideoSourceFileAudioCtrl::VideoSourceFileAudioCtrl() {
	CtrlLayout(*this);
	
}

void VideoSourceFileAudioCtrl::Data() {
	
}

void VideoSourceFileAudioCtrl::ToolMenu(Bar& bar) {
	
}

INITIALIZER_COMPONENT_CTRL(VideoSourceFileAudio, VideoSourceFileAudioCtrl)




END_UPP_NAMESPACE
