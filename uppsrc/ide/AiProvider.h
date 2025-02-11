#ifndef _ide_AiProvider_h_
#define _ide_AiProvider_h_

NAMESPACE_UPP

struct AiServiceProvider {
	typedef enum : int {
		UNDEFINED,
		OPENAI,
		API_OPENAI_COMPLETION,
		API_OPENAI_CHAT,
		API_OPENAI_IMAGE,
		API_OPENAI_VISION,
		API_WHISPERFILE_TRANSCRIPT,
		
		TYPE_COUNT
	} Type;
	
	static String GetTypeString(int i) {
		switch(i) {
			case UNDEFINED:						return "";
			case OPENAI:						return "OpenAI";
			case API_OPENAI_COMPLETION:			return "URL+API(OpenAI): Completion";
			case API_OPENAI_CHAT:				return "URL+API(OpenAI): Chat";
			case API_OPENAI_IMAGE:				return "URL+API(OpenAI): Image";
			case API_OPENAI_VISION:				return "URL+API(OpenAI): Vision";
			case API_WHISPERFILE_TRANSCRIPT:	return "URL+API(Whisperfile): Transcript";
			default: return "<error>";
		}
	};
	Type type = UNDEFINED;
	String name, proxy, url, token;
	int priority = 0;
	
	void Serialize(Stream& s) {
		int version = 1;
		s / version;
		if (version >= 1)
			s % (int&)type % name % proxy % url % token % priority;
	}
	
	String GetTypeString() const {return GetTypeString(type);}
	bool IsApiOpenAICompletion() const {return type == OPENAI || type == API_OPENAI_COMPLETION;}
	bool IsApiOpenAIChat() const {return type == OPENAI || type == API_OPENAI_CHAT;}
	bool IsApiOpenAIImage() const {return type == OPENAI || type == API_OPENAI_IMAGE;}
	bool IsApiOpenAIVision() const {return type == OPENAI || type == API_OPENAI_VISION;}
};


class AiServiceProviderManager {
	Array<AiServiceProvider> providers;
public:
	typedef AiServiceProviderManager CLASSNAME;
	AiServiceProviderManager();
	AiServiceProvider& Add();
	void Remove(int i);
	AiServiceProvider& operator[](int i);
	int GetCount() const;
	void Serialize(Stream& s) {s % providers;}
};

AiServiceProviderManager& AiManager();

END_UPP_NAMESPACE

#endif
