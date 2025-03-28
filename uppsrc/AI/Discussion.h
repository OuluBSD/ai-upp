#ifndef _AI_Discussion_h_
#define _AI_Discussion_h_


NAMESPACE_UPP


class AiDiscussionManager : public DiscussionManager {
	
public:
	typedef AiDiscussionManager CLASSNAME;
	AiDiscussionManager();
	
	void OnPhraseEnd(SoundPhrase& p) override;
	void OnMessageEnd(SoundMessage&) override;
	void OnDiscussionEnd(SoundDiscussion&) override;
	
	static AiDiscussionManager& Single() {static AiDiscussionManager m; return m;}
};

END_UPP_NAMESPACE


#endif
