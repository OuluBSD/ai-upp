#ifndef _AI_Omni_h_
#define _AI_Omni_h_


NAMESPACE_UPP

/*
ChatAgent
	- inter-agent-communication
ChatDirectorAgent
*/
class ChatThread {
	
public:
	typedef ChatThread CLASSNAME;
	
};

class SpeechTranscriptionThread {
	
public:
	typedef SpeechTranscriptionThread CLASSNAME;
	
};

class SpeechGenerationThread {
	
public:
	typedef SpeechGenerationThread CLASSNAME;
	
};

class ImageGenerationThread {
	
public:
	typedef ImageGenerationThread CLASSNAME;
	
};

class ImageVisionThread {
	
public:
	typedef ChatThread CLASSNAME;
	
};

class MetaEnvThread {
	
public:
	typedef MetaEnvThread CLASSNAME;
	
};

class OmniThread :
	public ChatThread,
	public SpeechTranscriptionThread,
	public SpeechGenerationThread,
	public ImageGenerationThread,
	public ImageVisionThread,
	public MetaEnvThread,
	public SoundDiscussionManager
{
	
public:
	typedef OmniThread CLASSNAME;
	OmniThread();
	
	void OnPhraseEnd(SoundPhrase& p) override;
	void OnMessageEnd(SoundMessage&) override;
	void OnDiscussionEnd(SoundDiscussion&) override;
	
	static OmniThread& Single() {static OmniThread m; return m;}
};

END_UPP_NAMESPACE


#endif
