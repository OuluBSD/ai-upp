#ifndef _AI_Omni_h_
#define _AI_Omni_h_


NAMESPACE_UPP

struct AiThread : Pte<AiThread> {
	virtual ~AiThread() {}
};

class CompletionThread : public virtual AiThread {
	
public:
	typedef CompletionThread CLASSNAME;
	
};

/*
ChatAgent
	- inter-agent-communication
ChatDirectorAgent
*/
class ChatThread : public virtual AiThread {
	
public:
	typedef ChatThread CLASSNAME;
	
};

class SpeechTranscriptionThread : public virtual AiThread {
	
public:
	typedef SpeechTranscriptionThread CLASSNAME;
	
};

class SpeechGenerationThread : public virtual AiThread {
	
public:
	typedef SpeechGenerationThread CLASSNAME;
	
};

class ImageGenerationThread : public virtual AiThread {
	
public:
	typedef ImageGenerationThread CLASSNAME;
	
};

class ImageVisionThread : public virtual AiThread {
	
public:
	typedef ChatThread CLASSNAME;
	
};

class MetaEnvThread : public virtual AiThread {
	
public:
	typedef MetaEnvThread CLASSNAME;
	
};

class OmniThread :
	public CompletionThread,
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
