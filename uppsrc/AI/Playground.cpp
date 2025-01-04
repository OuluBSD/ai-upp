#include "AI.h"

NAMESPACE_UPP


CompletionCtrl::CompletionCtrl() {
	CtrlLayout(*this);
	
}

TextToSpeechCtrl::TextToSpeechCtrl() {
	CtrlLayout(*this);
	
}

AssistantCtrl::AssistantCtrl() {
	CtrlLayout(*this);
	
}

RealtimeAiCtrl::RealtimeAiCtrl() {
	CtrlLayout(*this);
	
}

ChatAiCtrl::ChatAiCtrl() {
	CtrlLayout(*this);
	
}

CustomBiasesCtrl::CustomBiasesCtrl() {
	CtrlLayout(*this);
	
}

PlaygroundCtrl::PlaygroundCtrl() {
	Add(tabs.SizePos());
	
	tabs.Add(completion.SizePos(), "Completion");
	tabs.Add(tts.SizePos(), "TTS");
	tabs.Add(ass.SizePos(), "Assistant");
	tabs.Add(rt.SizePos(), "Realtime");
	tabs.Add(chat.SizePos(), "Chat");
	tabs.Add(bias.SizePos(), "Custom Biases");
	tabs.Add(edit_img.SizePos(), "Image Generation");
	tabs.Add(img_aspect.SizePos(), "Image Aspect Fixer");
	
	/*
	TODO
	
	void Data();
	void ToolMenu(Bar& bar);
	*/
}


END_UPP_NAMESPACE
