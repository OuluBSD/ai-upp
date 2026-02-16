#ifndef _Maestro_QuotaManager_h_
#define _Maestro_QuotaManager_h_

class QuotaManager {
	struct ModelStatus : Moveable<ModelStatus> {
		Time exhausted_until;
		
		void Jsonize(JsonIO& io) { io("exhausted_until", exhausted_until); }
	};
	
	static VectorMap<String, ModelStatus> statuses;
	static Mutex                          mutex;
	static bool                           loaded;

	static String GetConfigPath();
	static void   Load();
	static void   Save();

public:
	static bool   IsModelExhausted(const String& model);
	static void   MarkModelExhausted(const String& model, int hours = 1);
	static String GetBestGeminiModel();
	static void   Reset();
};

#endif
