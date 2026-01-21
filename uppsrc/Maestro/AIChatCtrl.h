#ifndef _Maestro_AIChatCtrl_h_
#define _Maestro_AIChatCtrl_h_

struct TodoItem : Moveable<TodoItem> {
	String id;
	String content;
	String status;
};

class TodoManager {
public:
	Array<TodoItem> todos;
	Ctrl* ctrl = nullptr;

	void ParseFromJson(const String& jsonStr);
	void Refresh();
	void SetCtrl(Ctrl* c);
};

extern TodoManager todo_manager;

class MaestroTodoList : public Ctrl {
public:
	virtual void Paint(Draw& d) override;
};

class MaestroItem : public Ctrl {
public:
	String role;
	String text;
	bool   is_error = false;
	bool   is_tool = false;
	
	virtual void Paint(Draw& d) override;
	int GetHeight(int width) const;
	
	typedef MaestroItem CLASSNAME;
	MaestroItem() {}
};

class AIChatCtrl : public Ctrl {
	Array<MaestroItem> items;
	String             current_response;
	ScrollBar          vscroll;
	
	String             queued_prompt;
	bool               waiting_to_send = false;
	
	// User additions
	ParentCtrl      chat;
	MaestroTodoList todo;
	Switch          send_continue;
	
public:
	EditString         input;
	Button             send;
	String             backend;
	CliMaestroEngine   engine;
	MaestroToolRegistry tools;
	
	void OnSend();
	void OnEvent(const MaestroEvent& e);
	void Poll();
	
	void AddItem(const String& role, const String& text, bool is_error = false);
	void AddToolItem(const String& role, const String& text);
	void CopyAllChat();
	void CopyDebugData();
	void OnSelectSession();
	void OnDone(bool result, bool fail);
	
	String GetResponse() const { return current_response; }

	virtual void Layout() override;
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) override;

	typedef AIChatCtrl CLASSNAME;
	AIChatCtrl();
	
	Function<void()> WhenDone;
	Function<void(const MaestroEvent& e)> WhenEvent;
};

#endif
