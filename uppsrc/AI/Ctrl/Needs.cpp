#include "Ctrl.h"
#define PREF(obj) auto& obj = p.obj;
#define REF(obj) auto& obj = p.needs.obj;
#define ALL_REFS \
	PREF(platforms); \
	REF(vsplit); \
	REF(rolesplit); \
	REF(platsplit); \
	REF(eventsplit); \
	REF(roles); \
	REF(needs); \
	REF(causes); \
	REF(messages); \
	REF(actions); \
	REF(action_causes); \
	REF(events); \
	REF(entries); \
	REF(event); \
	REF(entry); \
	 
NAMESPACE_UPP


void BiographyPlatformCtrl::Platforms_Needs_Ctor() {
	ALL_REFS
	this->p.tabs.Add(vsplit.VSizePos(0,20).HSizePos(), "Needs");
	
	vsplit.Vert() << rolesplit << platsplit << eventsplit;
	
	rolesplit.Horz() << roles << needs << causes << messages;
	platsplit.Horz() << platforms << actions << action_causes;
	eventsplit.Horz() << events << event << entries << entry;
	
	roles.AddColumn(t_("Role"));
	roles.AddIndex("IDX");
	roles.WhenCursor << THISBACK(Platforms_Needs_DataRole);
	roles.WhenBar << [this](Bar& b) {
		b.Add("Add role", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!p.owner) return;
			String role;
			bool b = EditTextNotNull(
				role,
				t_("Add Role"),
				t_("Role's name"),
				0
			);
			if (!b) return;
			if (p.owner->FindRole(role) >= 0) {
				PromptOK(t_("Role already exists: ") + role);
				return;
			}
			p.owner->roles.Add().name = role;
			PostCallback(THISBACK(Platforms_Needs_DataPlatform));
		});
		b.Add("Remove role", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!roles.IsCursor()) return;
			int cur = roles.Get("IDX");
			if (cur >= 0 && cur < p.owner->roles.GetCount())
				p.owner->roles.Remove(cur);
			PostCallback(THISBACK(Platforms_Needs_DataPlatform));
		});
		
	};
	
	needs.AddColumn(t_("Need"));
	needs.AddIndex("IDX");
	needs.WhenCursor << THISBACK(Platforms_Needs_DataNeed);
	
	causes.AddColumn(t_("Cause"));
	causes.AddIndex("IDX");
	
	messages.AddColumn(t_("Message"));
	messages.AddIndex("IDX");
	
	platforms.AddColumn(t_("Platform"));
	platforms.AddIndex("IDX");
	
	actions.AddColumn(t_("Action"));
	actions.AddIndex("IDX");
	actions.WhenCursor << THISBACK(Platforms_Needs_DataAction);
	actions.WhenBar << [this](Bar& b) {
		b.Add("Add action", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor()) return;
			int role_i = roles.Get("IDX");
			String action;
			bool b = EditTextNotNull(
				action,
				t_("Add action"),
				t_("Action's name"),
				0
			);
			if (!b) return;
			if (p.owner->roles[role_i].FindAction(action) >= 0) {
				PromptOK(t_("Action already exists: ") + action);
				return;
			}
			p.owner->roles[role_i].actions.Add().name = action;
			PostCallback(THISBACK(Platforms_Needs_DataRole));
		});
		b.Add("Remove action", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			p.owner->roles[role_i].actions.Remove(action_i);
			PostCallback(THISBACK(Platforms_Needs_DataRole));
		});
		
	};
	
	action_causes.AddColumn(t_("Need"));
	action_causes.AddColumn(t_("Cause"));
	action_causes.ColumnWidths("1 2");
	action_causes.AddIndex("IDX");
	
	events.AddColumn(t_("Event"));
	events.AddIndex("IDX");
	events.WhenBar << [this](Bar& b) {
		b.Add("Add event", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			p.owner->roles[role_i].actions[action_i].events.Add();
			PostCallback(THISBACK(Platforms_Needs_DataRole));
		});
		b.Add("Remove event", [this]() {
			ALL_REFS
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor() || !events.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			int event_i = events.Get("IDX");
			p.owner->roles[role_i].actions[action_i].events.Remove(event_i);
			PostCallback(THISBACK(Platforms_Needs_DataRole));
		});
		
	};
	
	entries.AddColumn(t_("Platform"));
	entries.AddColumn(t_("Entry"));
	entries.ColumnWidths("1 4");
	entries.AddIndex("IDX");
	entries.WhenCursor << THISBACK(Platforms_Needs_DataEntry);
	
	event.WhenAction << [this]() {
		ALL_REFS
		DatasetPtrs p = GetDataset();
		if (!p.owner || !roles.IsCursor() || !actions.IsCursor() || !events.IsCursor()) return;
		int role_i = roles.Get("IDX");
		int need_i = needs.Get("IDX");
		int action_i = actions.Get("IDX");
		int event_i = events.Get("IDX");
		String s = event.GetData();
		p.owner->roles[role_i].actions[action_i].events[event_i].text = s;
		s.Replace("\n", "\\n");
		events.Set(0, s);
		//PostCallback(THISBACK(DataEvent));
	};
	
}

void BiographyPlatformCtrl::Platforms_Needs_DataPlatform() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner) return;
	
	for(int i = 0; i < p.owner->roles.GetCount(); i++) {
		Role& r = p.owner->roles[i];
		roles.Set(i, 0, r.name);
		roles.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR(roles);
	roles.SetCount(p.owner->roles.GetCount());
	roles.SetSortColumn(0);
	if (!roles.IsCursor() && roles.GetCount())
		roles.SetCursor(0);
	
	Platforms_Needs_DataRole();
}

void BiographyPlatformCtrl::Platforms_Needs_DataRole() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner || !roles.IsCursor())
		return;
	
	int role_i = roles.Get("IDX");
	Role& r = p.owner->roles[role_i];
	
	for(int i = 0; i < r.needs.GetCount(); i++) {
		Need& n = r.needs[i];
		needs.Set(i, 0, n.name);
		needs.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(needs, a);
	needs.SetCount(r.needs.GetCount());
	needs.SetSortColumn(0);
	if (!needs.IsCursor() && needs.GetCount())
		needs.SetCursor(0);
	
	for(int i = 0; i < r.actions.GetCount(); i++) {
		RoleAction& ra = r.actions[i];
		actions.Set(i, 0, ra.name);
		actions.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(actions, b);
	actions.SetCount(r.actions.GetCount());
	actions.SetSortColumn(0);
	if (!actions.IsCursor() && actions.GetCount())
		actions.SetCursor(0);
	
	Platforms_Needs_DataNeed();
	Platforms_Needs_DataAction();
	Platforms_Needs_DataEvent();
}

void BiographyPlatformCtrl::Platforms_Needs_DataNeed() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner || !roles.IsCursor() || !needs.IsCursor())
		return;
	
	int role_i = roles.Get("IDX");
	int need_i = needs.Get("IDX");
	Role& r = p.owner->roles[role_i];
	Need& n = r.needs[need_i];
	
	for(int i = 0; i < n.causes.GetCount(); i++) {
		String& s = n.causes[i];
		causes.Set(i, 0, s);
		causes.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(causes, a);
	causes.SetCount(n.causes.GetCount());
	causes.SetSortColumn(0);
	if (!causes.IsCursor() && causes.GetCount())
		causes.SetCursor(0);
	
	for(int i = 0; i < n.messages.GetCount(); i++) {
		String& s = n.messages[i];
		messages.Set(i, 0, s);
		messages.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(messages, b);
	messages.SetCount(n.messages.GetCount());
	messages.SetSortColumn(0);
	if (!messages.IsCursor() && messages.GetCount())
		messages.SetCursor(0);
	
	TODO
	#if 0
	const auto& plats = GetPlatforms();
	int row = 0;
	for(int i = 0; i < n.platforms.GetCount(); i++) {
		if (n.platforms[i].enabled) {
			platforms.Set(row, 0, plats[i].name);
			platforms.Set(row, "IDX", i);
			row++;
		}
	}
	INHIBIT_CURSOR_(platforms, c);
	platforms.SetCount(row);
	platforms.SetSortColumn(0);
	if (!platforms.IsCursor() && platforms.GetCount())
		platforms.SetCursor(0);
	#endif
}

void BiographyPlatformCtrl::Platforms_Needs_DataAction() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner || !roles.IsCursor() || !actions.IsCursor()) {
		action_causes.Clear();
		events.Clear();
		entries.Clear();
		event.SetData("");
		entry.SetData("");
		return;
	}
	
	int role_i = roles.Get("IDX");
	int action_i = actions.Get("IDX");
	Role& r = p.owner->roles[role_i];
	RoleAction& ra = r.actions[action_i];
	
	int row = 0;
	for(int i = 0; i < ra.need_causes.GetCount(); i++) {
		const auto& nc = ra.need_causes[i];
		if (nc.need_i >= r.needs.GetCount()) continue;
		Need& n = r.needs[nc.need_i];
		if (nc.cause_i >= n.causes.GetCount()) continue;
		String& cause = n.causes[nc.cause_i];
		action_causes.Set(i, 0, n.name);
		action_causes.Set(i, 1, cause);
		action_causes.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(action_causes, a);
	action_causes.SetCount(ra.need_causes.GetCount());
	action_causes.SetSortColumn(0);
	if (!action_causes.IsCursor() && action_causes.GetCount())
		action_causes.SetCursor(0);
	
	for(int i = 0; i < ra.events.GetCount(); i++) {
		RoleEvent& re = ra.events[i];
		String s = re.text;
		s.Replace("\n", "\\n");
		events.Set(i, 0, s);
		events.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR_(events, c);
	events.SetCount(ra.events.GetCount());
	events.SetSortColumn(0);
	if (!events.IsCursor() && events.GetCount())
		events.SetCursor(0);
	
	Platforms_Needs_DataEvent();
}

void BiographyPlatformCtrl::Platforms_Needs_DataEvent() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner || !roles.IsCursor() || !events.IsCursor() || !actions.IsCursor()) {
		entries.Clear();
		event.SetData("");
		entry.SetData("");
		return;
	}
	
	int role_i = roles.Get("IDX");
	int event_i = events.Get("IDX");
	int action_i = actions.Get("IDX");
	Role& r = p.owner->roles[role_i];
	RoleAction& ra = r.actions[action_i];
	RoleEvent& re = ra.events[event_i];
	
	event.SetData(re.text);
	
	const auto& plats = GetPlatforms();
	for(int i = 0; i < re.entries.GetCount(); i++) {
		int plat_i = re.entries.GetKey(i);
		String plat_name = plats[plat_i].name;
		entries.Set(i, 0, plat_name);
		entries.Set(i, 1, re.entries[i]);
		entries.Set(i, "IDX", i);
	}
	INHIBIT_CURSOR(entries);
	entries.SetCount(re.entries.GetCount());
	if (entries.GetCount() && !entries.IsCursor())
		entries.SetCursor(0);
	
	Platforms_Needs_DataEntry();
}

void BiographyPlatformCtrl::Platforms_Needs_DataEntry() {
	ALL_REFS
	DatasetPtrs p = GetDataset();
	if (!p.owner || !roles.IsCursor() || !actions.IsCursor() || !events.IsCursor() || !entries.IsCursor()) return;
	int role_i = roles.Get("IDX");
	int need_i = needs.Get("IDX");
	int action_i = actions.Get("IDX");
	int event_i = events.Get("IDX");
	int entry_i = entries.Get("IDX");
	RoleEvent& re = p.owner->roles[role_i].actions[action_i].events[event_i];
	String& s = re.entries[entry_i];
	entry.SetData(s);
}

void BiographyPlatformCtrl::Platforms_Needs_ToolMenu(Bar& bar) {
	ALL_REFS
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Platforms_Needs_Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Platforms_Needs_Do, 1)).Key(K_F6);
	
}

void BiographyPlatformCtrl::Platforms_Needs_Do(int fn) {
	ALL_REFS
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	SocialNeedsProcess& ss = SocialNeedsProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}



END_UPP_NAMESPACE
