#include "Eon.h"


NAMESPACE_UPP




MAKE_STATIC(GlobalAudioTime, audtime);

GlobalAudioTime& GlobalAudioTime::Local() {
	return audtime;
}


END_UPP_NAMESPACE

