#ifndef _Sound_Discussion_h_
#define _Sound_Discussion_h_


NAMESPACE_UPP


struct SoundPhrase {
	SoundPhrase() {}
	
};

struct SoundMessage {
	Array<SoundPhrase> phrases;
	
	SoundMessage() {}
	
};

struct SoundDiscussion {
	Array<SoundMessage> messages;
	
	SoundDiscussion() {}
	
};

struct DiscussionManager {
	Array<SoundDiscussion> discussions;
	
	DiscussionManager() {}
	
	//DiscussionManager& Static() {static DiscussionManager d; return d;}
	
};

END_UPP_NAMESPACE


#endif
