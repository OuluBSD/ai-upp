#ifndef _AICtrl_ProtoVideoTask1_h_
#define _AICtrl_ProtoVideoTask1_h_




class ProtoVideoTask1Ctrl : public ComponentCtrl {
	PageCtrl pages;
	WithProtoVideoTask1_Input<Ctrl> input;
	WithProtoVideoTask1_TxtOut<Ctrl> sora_preset;
	WithProtoVideoTask1_TxtOut<Ctrl> slideshow_prompts;
	WithProtoVideoTask1_TxtOut<Ctrl> descriptions;
	TimeCallback tc;
	RunningFlagSingle flag;
	
public:
	typedef ProtoVideoTask1Ctrl CLASSNAME;
	ProtoVideoTask1Ctrl();
	~ProtoVideoTask1Ctrl();
	
	void Initialize(Value args) override;
	void Data() override;
	void ToolMenu(Bar& bar) override;
	void UpdateData();
	void OnChange();
	void Save();
	void Load();
	void DoSora();
	void DoSlideshowPrompts();
	void DoDescriptions();
	
};

INITIALIZE(ProtoVideoTask1Ctrl)


#endif
