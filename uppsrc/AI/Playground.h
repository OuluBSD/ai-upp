#ifndef _AI_Playground_h_
#define _AI_Playground_h_

NAMESPACE_UPP


struct AiThreadCtrlBase {
	Ptr<AiThread> ai_thrd;
	
	bool HasThread() const {return ai_thrd;}
	template <class T>
	T& CastThread() {
		// this function is required, bc OmniThread inherits virtual AiThread with Pte<AiThread>
		ASSERT(ai_thrd);
		if (!ai_thrd) throw Exc("No thread");
		T* t = dynamic_cast<T*>(&*ai_thrd);
		ASSERT(t);
		if (!t) throw Exc("Can't cast AiThread");
		return *t;
	}
	CompletionThread& GetCompletionThread() {return CastThread<CompletionThread>();}
	
	void SetThread(AiThread& t);
};

class CompletionCtrl : public WithCompletion<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef CompletionCtrl CLASSNAME;
	CompletionCtrl();
	
	void Submit();
	CompletionThread& Thread();
};

class TextToSpeechCtrl : public WithTTS<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef TextToSpeechCtrl CLASSNAME;
	TextToSpeechCtrl();
	
};

class AssistantCtrl : public WithAssistants<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef AssistantCtrl CLASSNAME;
	AssistantCtrl();
	
};

class RealtimeAiCtrl : public WithRealtimeAI<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef RealtimeAiCtrl CLASSNAME;
	RealtimeAiCtrl();
	
};

class ChatAiCtrl : public WithChatAI<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef ChatAiCtrl CLASSNAME;
	ChatAiCtrl();
	
};

class CustomBiasesCtrl : public WithCustomBiases<Ctrl>, public AiThreadCtrlBase {
	
public:
	typedef CustomBiasesCtrl CLASSNAME;
	CustomBiasesCtrl();
	
};

class PlaygroundCtrl : public Ctrl {
	TabCtrl tabs;
	CompletionCtrl completion;
	TextToSpeechCtrl tts;
	AssistantCtrl ass;
	RealtimeAiCtrl rt;
	ChatAiCtrl chat;
	CustomBiasesCtrl bias;
	EditImage edit_img;
	EditImage img_aspect;
	TaskCtrl tasks;
	Ctrl placeholder;
	
	One<OmniThread> omni;
	
public:
	typedef PlaygroundCtrl CLASSNAME;
	
	PlaygroundCtrl();
	~PlaygroundCtrl();
	void Data();
	void StoreThis();
	void LoadThis();
	void CreateThread();
	void SetThread(OmniThread& t);
	
};


class PlaygroundApp : public TopWindow {
	PlaygroundCtrl pg;
	MenuBar menu;
	
public:
	typedef PlaygroundApp CLASSNAME;
	PlaygroundApp();
	
	
};

void RunAiPlayground();

END_UPP_NAMESPACE

#endif
