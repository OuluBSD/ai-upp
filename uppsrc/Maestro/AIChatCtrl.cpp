#include "Maestro.h"

namespace Upp {

void MaestroItem::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, SColorPaper());
	
	Color clr = SColorText();
	if(is_error) clr = Red();
	else if(role == "User") clr = Blue();
	
	Font fnt = StdFont().Bold();
	d.DrawText(2, 2, role, fnt, clr);
	
	int ty = 2 + fnt.GetLineHeight();
	d.DrawText(5, ty, text, StdFont(), SColorText());
}

int MaestroItem::GetHeight(int width) const {
	// Very simple height calculation for now
	Font fnt = StdFont();
	int n = GetTextSize(text, fnt).cx;
	int rows = 1;
	if(width > 10) rows = (n / (width - 10)) + 1;
	return 4 + fnt.GetLineHeight() * (rows + 1);
}

AIChatCtrl::AIChatCtrl() {
	Add(menu);
	Add(vscroll);
	Add(input);
	Add(send.SetLabel("Send"));
	Add(engine_select);
	
	menu.Set([=](Bar& bar) { MainMenu(bar); });
	
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

void AIChatCtrl::MainMenu(Bar& bar) {
	Function<void(Bar&)> submenu = [=](Bar& bar) { EditMenu(bar); };
	bar.Sub("Edit", submenu);
}

void AIChatCtrl::EditMenu(Bar& bar) {
	bar.Add("Copy All Chat", [=] { CopyAllChat(); });
}

void AIChatCtrl::CopyAllChat() {
	String full_chat;
	for(const auto& item : items) {
		full_chat << item.role << ":\n" << item.text << "\n\n";
	}
	WriteClipboardText(full_chat);
}

void AIChatCtrl::Layout() {
	Size sz = GetSize();
	
	int menu_h = menu.GetStdHeight();
	menu.SetRect(0, 0, sz.cx, menu_h);
	
	int bottom_h = 100;
	
	// Layout bottom controls
	int bw = 150;
	int bh = 25;
	engine_select.SetRect(10, sz.cy - bh - 10, bw, bh);
	send.SetRect(sz.cx - 90, sz.cy - bh - 10, 80, bh);
	input.SetRect(10, sz.cy - bottom_h + 10, sz.cx - 20, 50);
	
	// Layout items
	int view_y = menu_h;
	int view_h = sz.cy - bottom_h - menu_h;
	
vscroll.SetRect(sz.cx - 16, view_y, 16, view_h);
	
	int y = view_y - vscroll.Get();
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
	if(e.type == "delta") {
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
	else if(e.type == "message" || e.type == "result") {
		if(!e.text.IsEmpty()) current_response = e.text;
		if(items.GetCount() > 0 && items.Top().role.StartsWith("AI")) {
			items.Top().text = current_response;
			items.Top().Refresh();
			Layout();
		} else {
			AddItem("AI (" + engine_select.GetValue().ToString() + ")", current_response);
		}
		current_response.Clear();
	}
	else if(e.type == "error") {
		AddItem("Error", e.text, true);
	}
}

void AIChatCtrl::Poll() {
	engine.Do();
}

}