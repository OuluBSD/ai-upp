#ifndef _AI_Playground_h_
#define _AI_Playground_h_

NAMESPACE_UPP


class CompletionCtrl : public WithCompletion<Ctrl> {
	
public:
	typedef CompletionCtrl CLASSNAME;
	CompletionCtrl();
	
};

class TextToSpeechCtrl : public WithTTS<Ctrl> {
	
public:
	typedef TextToSpeechCtrl CLASSNAME;
	TextToSpeechCtrl();
	
};

class AssistantCtrl : public WithAssistants<Ctrl> {
	
public:
	typedef AssistantCtrl CLASSNAME;
	AssistantCtrl();
	
};

class RealtimeAiCtrl : public WithRealtimeAI<Ctrl> {
	
public:
	typedef RealtimeAiCtrl CLASSNAME;
	RealtimeAiCtrl();
	
};

class ChatAiCtrl : public WithChatAI<Ctrl> {
	
public:
	typedef ChatAiCtrl CLASSNAME;
	ChatAiCtrl();
	
};

class CustomBiasesCtrl : public WithCustomBiases<Ctrl> {
	
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
	
public:
	typedef PlaygroundCtrl CLASSNAME;
	
	PlaygroundCtrl();
	
};

END_UPP_NAMESPACE

#endif
