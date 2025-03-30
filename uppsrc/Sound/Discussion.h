#ifndef _Sound_Discussion_h_
#define _Sound_Discussion_h_


NAMESPACE_UPP

struct DiscussionManager;
struct SoundDiscussion;
struct SoundMessage;

struct SoundPhrase : Pte<SoundPhrase> {
	SoundMessage& owner;
	One<SoundClipBase> current;
	
	SoundPhrase(SoundMessage& o) : owner(o) {}
	
	void Finish();
	template<class T> void BeginClip(SoundClip<T>& clip) {
		current = new SoundClip<T>(clip);
		
	}
};

struct SoundMessage : Pte<SoundMessage> {
	SoundDiscussion& owner;
	Array<SoundPhrase> phrases;
	
	SoundMessage(SoundDiscussion& o) : owner(o) {}
	SoundPhrase& Add();
	void Finish();
	
};

struct SoundDiscussion : Pte<SoundDiscussion> {
	DiscussionManager& owner;
	Array<SoundMessage> messages;
	
	SoundDiscussion(DiscussionManager& o) : owner(o) {}
	SoundMessage& Add();
	void Finish();
	
};

struct DiscussionManager : Pte<DiscussionManager> {
	Array<SoundDiscussion> discussions;
	
	DiscussionManager() {}
	SoundDiscussion& Add();
	
	virtual void OnPhraseBegin(SoundPhrase&);
	virtual void OnMessageBegin(SoundMessage&);
	virtual void OnDiscussionBegin(SoundDiscussion&);
	
	virtual void OnPhraseEnd(SoundPhrase&) {}
	virtual void OnMessageEnd(SoundMessage&) {}
	virtual void OnDiscussionEnd(SoundDiscussion&) {}
	
	//DiscussionManager& Static() {static DiscussionManager d; return d;}
	
	Event<SoundPhrase&> WhenPhraseBegin;
	Event<SoundMessage&> WhenMessageBegin;
	Event<SoundDiscussion&> WhenDiscussionBegin;
};

END_UPP_NAMESPACE


#endif
