#ifndef _AI_Playground_h_
#define _AI_Playground_h_

NAMESPACE_UPP


struct AiThreadCtrlBase : Ctrl {
	struct Model : Moveable<Model> {
		String name;
		bool use_chat = false;
	};
	
	Ptr<AiThread> ai_thrd;
	Vector<Model> models;
	Ptr<MetaNode> node;
	
	// Persistent
	int model_i = -1;
	
	void Visit(NodeVisitor& s);
	int GetModelCount(bool use_chat);
	bool CannotDoCompletion(String model_name);
	void UpdateCompletionModels();
	void UpdateChatModels();
	void UpdateModels(bool completion);
	void AddModel(String name, bool use_chat=false);
	hash_t GetHashValue() const {return GetVisitJsonHash(*this);}
	
	virtual ~AiThreadCtrlBase(){}
	virtual Ctrl* GetCtrl() = 0;
	virtual void Data() = 0;
	virtual void Submit() {};
	virtual void MainMenu(Bar& bar);
	
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
	ChatThread& GetChatThread() {return CastThread<ChatThread>();}
	
	void SetThread(AiThread& t);
	void SetNode(MetaNode& n);
	
};

struct AiThreadExt : MetaExtCtrl {
	StageThread& GetStageThread();
	
};

class CompletionCtrl : public WithCompletion<AiThreadCtrlBase> {
	
public:
	typedef CompletionCtrl CLASSNAME;
	CompletionCtrl();
	
	void Data() override;
	void Submit() override;
	Ctrl* GetCtrl() override {return this;}
	CompletionThread& Thread();
};

class AiStageCtrl : public AiThreadExt {
	Splitter hsplit, rsplit;
	ArrayCtrl session;
	TreeCtrl structure;
	Vector<MetaNode*> sessions;
	VectorMap<int,MetaNode*> structure_nodes;
	
public:
	typedef AiStageCtrl CLASSNAME;
	AiStageCtrl();
	
	void Data() override;
	void DataSession();
	void DataItem();
	void ToolMenu(Bar& bar) override;
	void SaveTemplate();
	void LoadTemplate(MetaNode* n);
	void RemoveTemplate(MetaNode* n);
	void VisitNode(int tree_i, MetaNode& n);
	
	void SessionMenu(Bar& b);
	void AddSession();
	void RemoveSession();
	void RenameSession();
	void SetSessionVersion();
	void DuplicateSession();
	
	void StageMenu(Bar& b);
	void AddStageNode(int kind);
	void RenameStageNode();
	void RemoveStageNode();
	
};

INITIALIZE(AiStageCtrl)

class TextToSpeechCtrl : public WithTTS<AiThreadCtrlBase> {
	
public:
	typedef TextToSpeechCtrl CLASSNAME;
	TextToSpeechCtrl();
	
	void Data() override;
	Ctrl* GetCtrl() override {return this;}
};

class AssistantCtrl : public WithAssistants<AiThreadCtrlBase> {
	
public:
	typedef AssistantCtrl CLASSNAME;
	AssistantCtrl();
	
	void Data() override;
	Ctrl* GetCtrl() override {return this;}
};

class RealtimeAiCtrl : public WithRealtimeAI<AiThreadCtrlBase> {
	
public:
	typedef RealtimeAiCtrl CLASSNAME;
	RealtimeAiCtrl();
	
	void Data() override;
	Ctrl* GetCtrl() override {return this;}
};

class ChatAiCtrl : public WithChatAI<AiThreadCtrlBase> {
	int session_i = -1;
	ChatCtrl chat;
	
public:
	typedef ChatAiCtrl CLASSNAME;
	ChatAiCtrl();
	void Submit() override;
	void Data() override;
	void DataSession();
	void ClearSessionCtrl();
	void AddSession();
	void RemoveSession();
	void ClearSession();
	void MainMenu(Bar& bar) override;
	void SessionMenu(Bar& bar);
	Ctrl* GetCtrl() override {return this;}
};

class CustomBiasesCtrl : public WithCustomBiases<AiThreadCtrlBase> {
	
public:
	typedef CustomBiasesCtrl CLASSNAME;
	CustomBiasesCtrl();
	
	void Data() override;
	Ctrl* GetCtrl() override {return this;}
};

class PlaygroundCtrl : public Ctrl {
	TabCtrl tabs;
	CompletionCtrl completion;
	AiStageCtrl stage;
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
	void Visit(NodeVisitor&);
	void Data();
	void StoreThis();
	void LoadThis();
	void CreateThread();
	void SetThread(OmniThread& t);
	void SetNode(MetaNode& n);
	void TabMenu(Bar& bar);
	
	Event<> WhenTab;
	
};


class PlaygroundApp : public TopWindow {
	PlaygroundCtrl pg;
	MenuBar menu;
	One<MetaNode> omni_node;
	
public:
	typedef PlaygroundApp CLASSNAME;
	PlaygroundApp();
	~PlaygroundApp();
	
	void UpdateMenu();
	void MainMenu(Bar& bar);
	
};

void RunAiPlayground();

END_UPP_NAMESPACE

#endif
