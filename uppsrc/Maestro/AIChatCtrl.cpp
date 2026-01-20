#include "Maestro.h"

namespace Upp {

void MaestroItem::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, is_tool ? Color(240, 240, 240) : SColorPaper());
	
	// Background for role header
	d.DrawRect(0, 0, sz.cx, 20, SColorFace());
	
	Color clr = SColorText();
	if(is_error) clr = Red();
	else if(role == "User") clr = Blue();
	else if(is_tool) clr = Color(0, 120, 0);
	
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
				// Find last space to avoid mid-word break
				const char *space = NULL;
				for(int i = 0; i < n; i++) if(s[i] == ' ') space = s + i;
				if(space) sn = (int)(space - s) + 1;
			}
			
			d.DrawText(5, ty, s, tfnt, SColorText(), sn);
			s += sn;
			ty += line_h;
			if(ty > sz.cy) break;
		}
		if(ty > sz.cy) break;
	}
}

int MaestroItem::GetHeight(int width) const {
	int ty = 22;
	Font tfnt = is_tool ? Courier(StdFont().GetHeight()) : StdFont();
	int line_h = tfnt.GetLineHeight();
	int w = width - 10;
	
	if(w < 20) return ty + line_h;
	
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
	
	return ty + count * line_h + 4;
}

AIChatCtrl::AIChatCtrl() {
	Add(vscroll);
	Add(input);
	Add(send.SetLabel("Send"));
	Add(engine_select);
	
	engine_select.Add("gemini", "Gemini");
	engine_select.Add("qwen", "Qwen");
	engine_select.Add("claude", "Claude");
	engine_select.Add("codex", "Codex");
	engine_select.SetIndex(0);
	
	send << [=] { OnSend(); };
	
vscroll.WhenScroll = [=] { Layout(); };
	
	// Poll timer
	SetTimeCallback(-50, [=] { Poll(); }); // 50ms poll
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
	int idx = engine_select.GetIndex();
	String key = engine_select.GetKey(idx);
	
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
	int bottom_h = 100;
	
	// Layout bottom controls
	int bw = 150;
	int bh = 25;
	engine_select.SetRect(10, sz.cy - bh - 10, bw, bh);
	send.SetRect(sz.cx - 90, sz.cy - bh - 10, 80, bh);
	input.SetRect(10, sz.cy - bottom_h + 10, sz.cx - 20, 50);
	
	// Layout items
	int view_y = 0;
	int view_h = sz.cy - bottom_h;
	
vscroll.SetRect(sz.cx - 16, 0, 16, view_h);
	
	int y = -vscroll.Get();
	int w = sz.cx - 16;
	
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

void AIChatCtrl::AddItem(const String& role, const String& text, bool is_error) {
	MaestroItem& item = items.Add();
	item.role = role;
	item.text = text;
	item.is_error = is_error;
	item.is_tool = false;
	Add(item);
	Layout();
	vscroll.End();
}

void AIChatCtrl::AddToolItem(const String& role, const String& text) {
	MaestroItem& item = items.Add();
	item.role = role;
	item.text = text;
	item.is_error = false;
	item.is_tool = true;
	Add(item);
	Layout();
	vscroll.End();
}

void AIChatCtrl::OnSend() {
	String prompt = input.GetData();
	if(prompt.IsEmpty()) return;
	
	input.Clear();
	AddItem("User", prompt);
	
	current_response.Clear();
	
	// Configure engine
	int idx = engine_select.GetIndex();
	String key = engine_select.GetKey(idx);
	
	if(key == "gemini") ConfigureGemini(engine);
	else if(key == "qwen") ConfigureQwen(engine);
	else if(key == "claude") ConfigureClaude(engine);
	else if(key == "codex") ConfigureCodex(engine);
	
	engine.Send(prompt, [=](const MaestroEvent& e) { OnEvent(e); });
}

void AIChatCtrl::OnEvent(const MaestroEvent& e) {
	if(e.type == "tool_use") {
		AddToolItem("Tool Call: " + e.tool_name, e.tool_input);
	}
	else if(e.type == "tool_result") {
		AddToolItem("Tool Result: " + e.tool_name, e.text);
	}
	else if(e.delta) {
		if(e.role == "user") return;
		
		current_response << e.text;
		// Update last item if it's from AI
		if(items.GetCount() > 0 && items.Top().role.StartsWith("AI")) {
			items.Top().text = current_response;
			items.Top().Refresh();
			Layout();
		} else {
			AddItem("AI (" + engine_select.GetValue().ToString() + ")", current_response);
		}
	}
	else if(e.type == "message" || e.type == "assistant") {
		if(e.role == "user") return;
		
		if(!e.text.IsEmpty()) current_response = e.text;
		if(items.GetCount() > 0 && items.Top().role.StartsWith("AI")) {
			items.Top().text = current_response;
			items.Top().Refresh();
			Layout();
		} else {
			AddItem("AI (" + engine_select.GetValue().ToString() + ")", current_response);
		}
		
		if(e.role == "assistant" || e.type == "assistant") {
			current_response.Clear();
			WhenDone();
		}
	}
	else if(e.type == "result") {
		current_response.Clear();
		WhenDone();
	}
	else if(e.type == "turn.failed" || e.type == "error") {
		AddItem("Error", e.text, true);
		WhenDone();
	}
}

void AIChatCtrl::Poll() {
	engine.Do();
}

} // namespace Upp
