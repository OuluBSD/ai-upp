#include "Maestro.h"

#ifdef flagGUI

NAMESPACE_UPP

TodoManager todo_manager;

void TodoManager::ParseFromJson(const String& jsonStr) {
	todos.Clear();
	Value v = ParseJSON(jsonStr);
	if (v.IsError() || !v.Is<ValueArray>()) return;

	ValueArray arr = v;
	for (int i = 0; i < arr.GetCount(); i++) {
		Value item = arr[i];
		if (item.Is<ValueMap>()) {
			TodoItem t;
			t.id = item["id"].ToString();
			t.content = item["content"].ToString();
			t.status = item["status"].ToString();
			todos.Add(t);
		}
	}
}

void TodoManager::Refresh() {
	if (ctrl) ctrl->Refresh();
}

void TodoManager::SetCtrl(Ctrl* c) {
	ctrl = c;
}

MaestroItem::MaestroItem() {
	Add(approve.SetLabel("Approve").LeftPos(5, 80).BottomPos(5, 25));
	Add(reject.SetLabel("Reject").LeftPos(90, 80).BottomPos(5, 25));
	approve.Hide();
	reject.Hide();
	
	approve << [=] { if(WhenApprove) WhenApprove(); };
	reject  << [=] { if(WhenReject) WhenReject(); };
}

void MaestroItem::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, is_tool ? Blend(SColorPaper(), SColorText(), 50) : SColorPaper());
	
	// Background for role header
	d.DrawRect(0, 0, sz.cx, 20, SColorFace());
	
	Color clr = SColorText();
	if(is_error) clr = LtRed();
	else if(role == "User") clr = Blue();
	else if(is_tool) clr = LtGreen();
	
	Font fnt = StdFont().Bold();
	d.DrawText(2, 2, role, fnt, clr);
	
	int ty = 22;
	Font tfnt = is_tool ? Courier(StdFont().GetHeight()) : StdFont();
	int line_h = tfnt.GetLineHeight();
	int w = sz.cx - 10;
	
	if(w < 20) return;
	
	String txt = text;
	txt.Replace("\r", "");
	Vector<String> paragraphs = Split(txt, '\n', false);
	
	for(const String& p : paragraphs) {
		const char *s = p;
		if(!*s) { // Empty line
			ty += line_h;
			continue;
		}
		while(*s) {
			int n = 0;
			int total_w = 0;
			while(s[n] && total_w + tfnt[(byte)s[n]] <= w) {
				total_w += tfnt[(byte)s[n]];
				n++;
			}
			if(n == 0) break;
			
			int sn = n;
			if(n < (int)strlen(s)) {
				const char *space = NULL;
				for(int i = 0; i < n; i++) if(s[i] == ' ') space = s + i;
				if(space) sn = (int)(space - s) + 1;
			}
			
			d.DrawText(5, ty, s, tfnt, SColorText(), sn);
			s += sn;
			ty += line_h;
			if(ty > sz.cy - (pending ? 35 : 0)) break;
		}
		if(ty > sz.cy - (pending ? 35 : 0)) break;
	}
	
	if(pending) {
		approve.Show();
		reject.Show();
	} else {
		approve.Hide();
		reject.Hide();
	}
}

int MaestroItem::GetHeight(int width) const {
	int ty = 22;
	Font tfnt = is_tool ? Courier(StdFont().GetHeight()) : StdFont();
	int line_h = tfnt.GetLineHeight();
	int w = width - 10;
	
	if(w < 20) return ty + line_h + (pending ? 35 : 0);
	
	String txt = text;
	txt.Replace("\r", "");
	Vector<String> paragraphs = Split(txt, '\n', false);
	
	int count = 0;
	for(const String& p : paragraphs) {
		const char *s = p;
		if(!*s) {
			count++;
			continue;
		}
		while(*s) {
			int n = 0;
			int total_w = 0;
			while(s[n] && total_w + tfnt[(byte)s[n]] <= w) {
				total_w += tfnt[(byte)s[n]];
				n++;
			}
			if(n == 0) break;
			
			int sn = n;
			if(n < (int)strlen(s)) {
				const char *space = NULL;
				for(int i = 0; i < n; i++) if(s[i] == ' ') space = s + i;
				if(space) sn = (int)(space - s) + 1;
			}
			s += sn;
			count++;
		}
	}
	
	return ty + count * line_h + 4 + (pending ? 35 : 0);
}

AIChatCtrl::AIChatCtrl() {
	chat.AddFrame(vscroll);
	vscroll.Vert();
	
	int todo_height = 100;
	int edit_height = 100;
	int offset = 3;
	int btn_height = 30;
	int suggest_h = 40;
	
	Add(todo.TopPos(0,todo_height).HSizePos());
	Add(chat.VSizePos(todo_height, edit_height + offset + suggest_h).HSizePos());
	
	Add(suggestion.BottomPos(edit_height + offset, suggest_h).HSizePos());
	suggestion.Add(suggest_label.VSizePos().LeftPos(5, 400));
	suggestion.Add(enact_suggested.SetLabel("Enact Suggestion").VSizePos(5, 5).RightPos(5, 150));
	suggestion.Hide();
	
	Add(input.HSizePos(offset,100+offset).BottomPos(0,edit_height));
	
		Add(send_continue.RightPos(0,100+offset).BottomPos(2*btn_height,btn_height));
	
		Add(yolo_mode.RightPos(0,100+offset).BottomPos(1*btn_height,btn_height));
	
		Add(send.RightPos(0,100+offset).BottomPos(0*btn_height,btn_height));
	
		
	
send_continue.SetLabel("Auto-Continue");
	
		yolo_mode.SetLabel("YOLO Mode");
	
send.SetLabel("Send");
	
		
	
yolo_mode.Set(1);
	
	RegisterMaestroTools(tools);
	
	send.WhenAction = THISBACK(OnSend);
	vscroll.WhenScroll = [=] { Layout(); };
	vscroll.SetLine(30);
	
	SetTimeCallback(-50, [=] { Poll(); });
}

void AIChatCtrl::CopyAllChat() {
	String full_chat;
	for(const auto& item : items) {
		full_chat << item.role << ":\n" << item.text << "\n\n";
	}
	WriteClipboardText(full_chat);
}

void AIChatCtrl::CopyDebugData() {
	String debug;
	debug << "=== ENGINE LOG ===\n";
	debug << engine.debug_log << "\n";
	debug << "=== CHAT STATE ===\n";
	debug << "Items: " << items.GetCount() << "\n";
	for(int i = 0; i < items.GetCount(); i++) {
		debug << "[" << i << "] Role: " << items[i].role 
		      << " Len: " << items[i].text.GetCount() 
		      << " Error: " << items[i].is_error << " Tool: " << items[i].is_tool << "\n";
	}
	WriteClipboardText(debug);
}

void AIChatCtrl::OnSelectSession() {
	String key = backend;
	if(key == "gemini") ConfigureGemini(engine);
	else if(key == "qwen") ConfigureQwen(engine);
	else if(key == "claude") ConfigureClaude(engine);
	else if(key == "codex") ConfigureCodex(engine);

	SessionSelectWindow sw(engine);
	sw.DataDirectories();
	if(sw.Run() == IDOK) {
		engine.session_id = sw.selected_id;
		AddItem("System", "Resumed session: " + engine.session_id);
	}
}

void AIChatCtrl::Layout() {
	Size sz = GetSize();
	int view_h = chat.GetSize().cy;
	int y = -vscroll.Get();
	int w = sz.cx;
	int total_h = 0;
	for(int i = 0; i < items.GetCount(); i++) {
		int ih = items[i].GetHeight(w);
		items[i].SetRect(0, y, w, ih);
		y += ih;
		total_h += ih;
	}
	vscroll.SetTotal(total_h);
	vscroll.SetPage(view_h);
}

void AIChatCtrl::MouseWheel(Point p, int zdelta, dword keyflags) {
	vscroll.Wheel(zdelta);
}

void AIChatCtrl::Load(const String& root) {
	// Initialize with root
}

void AIChatCtrl::SetSession(const String& b, const String& id) {
	backend = b;
	engine.binary = b;
	engine.session_id = id;
}

void AIChatCtrl::AddItem(const String& role, const String& text, bool is_error) {
	MaestroItem& item = items.Add();
	item.role = role;
	item.text = text;
	item.is_error = is_error;
	item.is_tool = false;
	chat.Add(item);
	Layout();
	vscroll.End();
}

MaestroItem& AIChatCtrl::AddToolItem(const String& role, const String& text) {
	MaestroItem& item = items.Add();
	item.role = role;
	item.text = text;
	item.is_error = false;
	item.is_tool = true;
	chat.Add(item);
	Layout();
	vscroll.End();
	return item;
}

void AIChatCtrl::OnSend() {
	String prompt = input.GetData();
	if(prompt.IsEmpty()) return;
	if (engine.IsRunning()) {
		queued_prompt = prompt;
		waiting_to_send = true;
		return;
	}
	input.Clear();
	AddItem("User", prompt);
	current_response.Clear();
	String key = ToLower(backend);
	if(key == "gemini") ConfigureGemini(engine);
	else if(key == "qwen") ConfigureQwen(engine);
	else if(key == "claude") ConfigureClaude(engine);
	else if(key == "codex") ConfigureCodex(engine);
	engine.Send(prompt, [=](const MaestroEvent& e) { OnEvent(e); });
}

void AIChatCtrl::OnEvent(const MaestroEvent& e) {
	if(WhenEvent) WhenEvent(e);
	
	if(e.type == "tool_use") {
		if (e.tool_name == "todo_write") {
			ValueMap vm = e.json["input"];
			for(int i = 0; i < vm.GetCount(); i++) {
				if(vm.GetKey(i) == "todos") {
					todo_manager.ParseFromJson(vm.GetValue(i).ToString());
					todo_manager.SetCtrl(&todo);
					todo.Refresh();
					break;
				}
			}
		} else {
			const MaestroTool* t = tools.Find(e.tool_name);
			if(t) {
				MaestroItem& item = AddToolItem("Local Tool Call: " + e.tool_name, e.tool_input);
				
				auto ExecuteLocal = [this, &item, t, e]() {
					item.pending = false;
					item.Refresh();
					Layout();
					Value params = ParseJSON(e.tool_input);
					Value result = t->Execute(params.Is<ValueMap>() ? (ValueMap)params : ValueMap());
					AddToolItem("Local Tool Result: " + e.tool_name, AsString(result));
					engine.WriteToolResult(e.tool_id, result);
				};
				
				if(yolo_mode.GetData()) {
					ExecuteLocal();
				} else {
					item.pending = true;
					item.WhenApprove = ExecuteLocal;
					item.WhenReject = [this, &item, e]() {
						item.pending = false;
						item.is_error = true;
						item.Refresh();
						Layout();
						engine.WriteToolResult(e.tool_id, "Error: Tool execution rejected by user.");
					};
					item.Refresh();
					Layout();
				}
				return;
			}
			
			String role = "Tool Call: " + e.tool_name;
			if(items.GetCount() > 0 && items.Top().role == role) {
				items.Top().text = e.tool_input;
				items.Top().Refresh();
				Layout();
			} else {
				AddToolItem(role, e.tool_input);
			}
		}
	} 
	else if(e.type == "tool_result") {
		String role = "Tool Result: " + e.tool_name;
		if(items.GetCount() > 0 && items.Top().role == role) {
			items.Top().text = e.text;
			items.Top().Refresh();
			Layout();
		} else {
			AddToolItem(role, e.text);
		}
	}
	else if(e.delta) {
		if(e.role == "user") return;
		current_response << e.text;
		if(items.GetCount() > 0 && items.Top().role.StartsWith("AI") && !items.Top().is_tool) {
			items.Top().text = current_response;
			items.Top().Refresh();
			Layout();
		} else {
			AddItem("AI (" + backend + ")", current_response);
		}
	}
	else if(e.type == "message" || e.type == "assistant") {
		if(e.role == "user") return;
		if(!e.text.IsEmpty()) current_response = e.text;
		if(items.GetCount() > 0 && items.Top().role.StartsWith("AI") && !items.Top().is_tool) {
			items.Top().text = current_response;
			items.Top().Refresh();
			Layout();
		} else {
			AddItem("AI (" + backend + ")", current_response);
		}
		if(e.role == "assistant" || e.type == "assistant") OnDone(false, false);
	}
	else if(e.type == "result") OnDone(true, false);
	else if(e.type == "turn.failed" || e.type == "error") {
		AddItem("Error", e.text, true);
		OnDone(false, true);
	}
}

void AIChatCtrl::OnDone(bool result, bool fail) {
	if(WhenDone) WhenDone();
	current_response.Clear();
	if (waiting_to_send) {
		waiting_to_send = false;
		String temp_prompt = queued_prompt;
		queued_prompt.Clear();
		String saved_input = input.GetData();
		input.SetData(temp_prompt);
		OnSend();
		input.SetData(saved_input);
	}
	else if (result && send_continue.GetData()) {
		PostCallback([this]{
			input.SetData("continue");
			OnSend();
		});
	}
}

void MaestroTodoList::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, SColorPaper());
	d.DrawText(5, 5, "TODO List", StdFont().Bold(), Blue());

	int y_offset = 25;
	int item_height = 25;

	for (int i = 0; i < todo_manager.todos.GetCount(); i++) {
		const TodoItem& item = todo_manager.todos[i];
		Color clr = SColorText();
		if (item.status == "completed") clr = Green();
		else if (item.status == "in_progress") clr = Orange();
		else if (item.status == "pending") clr = LtGray();

		Rect status_rect(10, y_offset + 5, 20, y_offset + item_height - 5);
		if (item.status == "completed") d.DrawRect(status_rect, Green());
		else if (item.status == "in_progress") d.DrawRect(status_rect, Orange());
		else d.DrawRect(status_rect, LtGray());

		String display_text = "[" + item.id + "] " + item.content + " (" + item.status + ")";
		d.DrawText(30, y_offset, display_text, StdFont(), clr);
		y_offset += item_height;
	}
}

void AIChatCtrl::Poll() {
	bool running = engine.Do();
	if(!running && !engine.IsRunning() && items.GetCount() > 0 && !current_response.IsEmpty()) {
		OnDone(true, false);
	}
}

void AIChatCtrl::SuggestEnactment(String track, String phase, String task) {
	suggested_task.Clear();
	suggested_task.Add("track", track);
	suggested_task.Add("phase", phase);
	suggested_task.Add("task", task);
	
	suggest_label.SetText("Suggest next: **" + task + "**");
	suggestion.Show();
	Layout();
}

END_UPP_NAMESPACE

#endif