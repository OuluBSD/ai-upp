#ifndef _AI_Playground_h_
#define _AI_Playground_h_

NAMESPACE_UPP


struct AiThreadCtrlBase : VfsValueExtCtrl {
	struct Model : Moveable<Model> {
		String name;
		bool use_chat = false;
	};
	
	Vector<Model> models;
	Ptr<VfsValue> node;
	
	// Persistent
	int model_i = -1;
	
	void Visit(Vis& s);
	int GetModelCount(bool use_chat);
	bool CannotDoCompletion(String model_name);
	void UpdateCompletionModels();
	void UpdateChatModels();
	void UpdateModels(bool completion);
	void AddModel(String name, bool use_chat=false);
	hash_t GetHashValue() const {return GetVisitJsonHash(*this);}
	void SetNode(VfsValue& n);
	
	virtual ~AiThreadCtrlBase(){}
	virtual Ctrl* GetCtrl() = 0;
	virtual void Data() = 0;
	virtual void Submit() {};
	virtual void MainMenu(Bar& bar);
	
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

struct AiChainCtrl : VfsValueExtCtrl {
	
private:
	Splitter hsplit, msplit, rsplit;
	ArrayCtrl session;
	TreeCtrl structure;
	Vector<VfsValue*> sessions;
	VectorMap<int,VfsValue*> structure_nodes;
	VectorMap<int,String> structure_values;
	
public:
	typedef AiChainCtrl CLASSNAME;
	AiChainCtrl();
	
	void Data() override;
	void DataSession();
	void DataItem();
	void ToolMenu(Bar& bar) override;
	void VisitNode(int tree_i, VfsValue& n, String path);
	VfsValue* GetSession();
	
	void SessionMenu(Bar& b);
	void AddSession();
	void RemoveSession();
	void RenameSession();
	void SetSessionVersion();
	void DuplicateSession();
	
	void StageMenu(Bar& b);
	void AddStageNode(hash_t type_hash);
	void RenameStageNode();
	void RemoveStageNode();
	
};

INITIALIZE(AiChainCtrl)

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
	VfsProgramCtrl stage;
	AiChainCtrl chain;
	TextToSpeechCtrl tts;
	AssistantCtrl ass;
	RealtimeAiCtrl rt;
	ChatAiCtrl chat;
	CustomBiasesCtrl bias;
	EditImage edit_img;
	EditImage img_aspect;
	TaskCtrl tasks;
	Ctrl placeholder;
	
	Ptr<VfsValue> node;
	
public:
	typedef PlaygroundCtrl CLASSNAME;
	
	PlaygroundCtrl();
	~PlaygroundCtrl();
	void Visit(Vis&);
	void Data();
	void StoreThis();
	void LoadThis();
	void CreateThread();
	void SetNode(VfsValue& n);
	void TabMenu(Bar& bar);
	
	Event<> WhenTab;
	
};


class PlaygroundApp : public TopWindow {
	PlaygroundCtrl pg;
	MenuBar menu;
	Ptr<VfsValue> omni_node;
	
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
