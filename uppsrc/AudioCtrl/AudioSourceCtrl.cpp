#include "AudioCtrl.h"

NAMESPACE_UPP


INITBLOCK_(AudioSourceCtrl) {
	MakeInterfaceCtrlFactory<AudioSource, AudioSourceCtrl>();
}



AudioSourceCtrl::AudioSourceCtrl() {
	Add(s.SizePos());
	s.Background(Color(176, 237, 255));
}

void AudioSourceCtrl::SetInterface(ComponentBaseRef c, ExchangeProviderBaseRef b) {
	
}



END_UPP_NAMESPACE
