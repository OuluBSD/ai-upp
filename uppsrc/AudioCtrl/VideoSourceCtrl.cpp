#include "AudioCtrl.h"

NAMESPACE_UPP



INITBLOCK_(VideoSourceCtrl) {
	MakeInterfaceCtrlFactory<VideoSource, VideoSourceCtrl>();
}



VideoSourceCtrl::VideoSourceCtrl() {
	Add(audio.SizePos());
	
}

void VideoSourceCtrl::SetInterface(ComponentBaseRef c, ExchangeProviderBaseRef b) {
	
}



END_UPP_NAMESPACE
