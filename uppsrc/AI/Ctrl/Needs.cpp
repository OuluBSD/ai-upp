#include "Ctrl.h"

NAMESPACE_UPP


SocialNeedsCtrl::SocialNeedsCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	
	hsplit.Vert() << rolesplit << platsplit << eventsplit;
	
	rolesplit.Horz() << roles << needs << causes << messages;
	platsplit.Horz() << platforms << actions << action_causes;
	eventsplit.Horz() << events << event << entries << entry;
	
	roles.AddColumn(t_("Role"));
	roles.AddIndex("IDX");
	roles.WhenCursor << THISBACK(DataRole);
	roles.WhenBar << [this](Bar& b) {
		b.Add("Add role", [this]() {
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
			PostCallback(THISBACK(Data));
		});
		b.Add("Remove role", [this]() {
			DatasetPtrs p = GetDataset();
			if (!roles.IsCursor()) return;
			int cur = roles.Get("IDX");
			if (cur >= 0 && cur < p.owner->roles.GetCount())
				p.owner->roles.Remove(cur);
			PostCallback(THISBACK(Data));
		});
		
	};
	
	needs.AddColumn(t_("Need"));
	needs.AddIndex("IDX");
	needs.WhenCursor << THISBACK(DataNeed);
	
	causes.AddColumn(t_("Cause"));
	causes.AddIndex("IDX");
	
	messages.AddColumn(t_("Message"));
	messages.AddIndex("IDX");
	
	platforms.AddColumn(t_("Platform"));
	platforms.AddIndex("IDX");
	
	actions.AddColumn(t_("Action"));
	actions.AddIndex("IDX");
	actions.WhenCursor << THISBACK(DataAction);
	actions.WhenBar << [this](Bar& b) {
		b.Add("Add action", [this]() {
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
			PostCallback(THISBACK(DataRole));
		});
		b.Add("Remove action", [this]() {
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			p.owner->roles[role_i].actions.Remove(action_i);
			PostCallback(THISBACK(DataRole));
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
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			p.owner->roles[role_i].actions[action_i].events.Add();
			PostCallback(THISBACK(DataRole));
		});
		b.Add("Remove event", [this]() {
			DatasetPtrs p = GetDataset();
			if (!p.owner || !roles.IsCursor() || !actions.IsCursor() || !events.IsCursor()) return;
			int role_i = roles.Get("IDX");
			int action_i = actions.Get("IDX");
			int event_i = events.Get("IDX");
			p.owner->roles[role_i].actions[action_i].events.Remove(event_i);
			PostCallback(THISBACK(DataRole));
		});
		
	};
	
	entries.AddColumn(t_("Platform"));
	entries.AddColumn(t_("Entry"));
	entries.ColumnWidths("1 4");
	entries.AddIndex("IDX");
	entries.WhenCursor << THISBACK(DataEntry);
	
	event.WhenAction << [this]() {
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

void SocialNeedsCtrl::Data() {
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
	
	DataRole();
}

void SocialNeedsCtrl::DataRole() {
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
	
	DataNeed();
	DataAction();
	DataEvent();
}

void SocialNeedsCtrl::DataNeed() {
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

void SocialNeedsCtrl::DataAction() {
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
	
	DataEvent();
}

void SocialNeedsCtrl::DataEvent() {
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
	
	TODO
	#if 0
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
	#endif
	
	DataEntry();
}

void SocialNeedsCtrl::DataEntry() {
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

void SocialNeedsCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	
}

void SocialNeedsCtrl::Do(int fn) {
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




















ArrayMap<hash_t, SocialNeedsProcess>& __SocialNeedsProcesss() {
	static ArrayMap<hash_t, SocialNeedsProcess> map;
	return map;
}


SocialNeedsProcess::SocialNeedsProcess() {
	
}

SocialNeedsProcess& SocialNeedsProcess::Get(Profile& e, BiographySnapshot& snap) {
	String t = e.node.GetPath() + ": " + e.node.GetPath();
	hash_t h = t.GetHashValue();
	ArrayMap<hash_t, SocialNeedsProcess>& map = __SocialNeedsProcesss();
	int i = map.Find(h);
	if (i >= 0)
		return map[i];
	
	SocialNeedsProcess& ls = map.Add(h);
	TODO
	#if 0
	ASSERT(e.owner);
	ls.owner = e.owner;
	ls.profile = &e;
	ls.snap = &snap;
	ls.analysis = &snap.analysis;
	ls.biography = &snap.data;
	return ls;
	#endif
}

int SocialNeedsProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

int SocialNeedsProcess::GetBatchCount(int phase) const {
	
	if (phase == PHASE_PACK_ROLE_REACTIONS) {
		return analysis->profiles.GetCount();
	}
	else if (phase == PHASE_PACK_PLATFORM_REACTIONS) {
		return PLATFORM_COUNT;
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTIONS) {
		return PLATFORM_COUNT;
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_REFINEMENTS) {
		return PLATFORM_COUNT;
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_TRANSLATED) {
		return PLATFORM_COUNT;
	}
	return 1;
}

int SocialNeedsProcess::GetSubBatchCount(int phase, int batch) const {
	if (phase == PHASE_PACK_ROLE_REACTIONS) {
		return max(1, ranges.GetCount());
	}
	else if (phase == PHASE_PACK_PLATFORM_REACTIONS) {
		return max(1, ranges.GetCount());
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_REFINEMENTS) {
		return PLATDESC_LEN_COUNT;
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_TRANSLATED) {
		return PLATDESC_MODE_COUNT * PLATDESC_LEN_COUNT;
	}
	return 1;
}

void SocialNeedsProcess::DoPhase() {
	TODO
	#if 0
	MetaDatabase& db = MetaDatabase::Single();
	LeadData& sd = db.lead_data;
	LeadDataAnalysis& sda = db.lead_data.a;
	//sa = &sda.GetLeadEntityAnalysis(owner->name);
	int lng_i = db.GetLanguageIndex();

	
	if (phase == PHASE_PACK_ROLE_REACTIONS) {
		ProcessRoleReactions();
	}
	else if (phase == PHASE_PACK_PLATFORM_REACTIONS) {
		ProcessPlatformReactions();
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTIONS) {
		ProcessPlatformDescriptions();
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_REFINEMENTS) {
		ProcessPlatformDescriptionRefinements();
	}
	else if (phase == PHASE_PLATFORM_DESCRIPTION_TRANSLATED) {
		ProcessPlatformDescriptionTranslated();
	}
	#endif
}

void SocialNeedsProcess::OnBatchError() {
	LOG("error: OnBatchError");
	Sleep(2000);
	SetWaiting(0);
	NextBatch();
}

void SocialNeedsProcess::ProcessRoleReactions() {
	Biography& biography = *this->biography;
	BiographyAnalysis& analysis = *this->analysis;
	int role_i = batch;
	
	if (role_i >= analysis.profiles.GetCount()) {
		NextPhase();
		return;
	}
	
	if (analysis.GetRequiredRoles().Find(role_i) < 0) {
		NextBatch();
		return;
	}
	
	if (role_i >= analysis.roles.GetCount())
		analysis.roles.SetCount(role_i+1);
	BiographyRoleAnalysis& role = analysis.roles[role_i];
	const auto& profs = GetRoleProfile(role_i);
	auto& profiles = analysis.profiles[role_i];
	int c = min(profiles.GetCount(), profs.GetCount());
	
	if (sub_batch == 0) {
		ranges.Clear();
		CreateRange(0, c);
		//DUMPC(ranges);
	}
	if (sub_batch >= ranges.GetCount()) {
		NextBatch();
		return;
	}
	if (skip_ready && role.merged_biography_reactions.GetCount() && role.merged_biography_reactions[0].GetCount()) {
		NextBatch();
		return;
	}
	int range_i = ranges.GetCount() - 1 - sub_batch;
	
	if (role.merged_biography_reactions.GetCount() != ranges.GetCount())
		role.merged_biography_reactions.Clear();
	role.merged_biography_reactions.SetCount(ranges.GetCount());
	const String& range_output = role.merged_biography_reactions[range_i];
	
	if (skip_ready && range_output.GetCount()) {
		NextSubBatch();
		return;
	}
	
	SocialArgs args;
	args.fn = 4;
	Range& range = ranges[range_i];
	if (range.input[0] >= 0) {
		const String& r0 = role.merged_biography_reactions[range.input[0]];
		args.parts.Add("<merged reaction #1>", r0);
	}
	else {
		int begin = range.off;
		int len_2 = range.len / 2;
		int end = begin + len_2;
		for(int i = begin; i < end; i++) {
			BiographyProfileAnalysis& pa = profiles[i];
			args.parts.Add(Capitalize(profs[i].name), pa.biography_reaction);
		}
	}
	
	if (range.input[1] >= 0) {
		const String& r0 = role.merged_biography_reactions[range.input[1]];
		args.parts.Add("<merged reaction #2>", r0);
	}
	else {
		int len_2 = range.len / 2;
		int begin = range.off + len_2;
		int end = begin + range.len - len_2;
		for(int i = begin; i < end; i++) {
			BiographyProfileAnalysis& pa = profiles[i];
			args.parts.Add(Capitalize(profs[i].name), pa.biography_reaction);
		}
	}
	
	SetWaiting(1);
	TaskMgr& m = AiTaskManager();
	m.GetSocial(args, THISBACK(OnProcessRoleReactions));
}

void SocialNeedsProcess::OnProcessRoleReactions(String res) {
	Biography& biography = *this->biography;
	BiographyAnalysis& analysis = *this->analysis;
	int role_i = batch;
	int range_i = ranges.GetCount() - 1 - sub_batch;
	
	if (role_i >= analysis.roles.GetCount())
		analysis.roles.SetCount(role_i+1);
	BiographyRoleAnalysis& role = analysis.roles[role_i];
	String& s = role.merged_biography_reactions[range_i];
	
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	NextSubBatch();
	SetWaiting(0);
}

int SocialNeedsProcess::CreateRange(int off, int len) {
	if (len <= 1) return -1;
	int pos = ranges.GetCount();
	Range& range = ranges.Add();
	range.off = off;
	range.len = len;
	int len_2 = len / 2;
	if (len_2 == 0) return pos;
	ranges[pos].input[1] = CreateRange(off + len_2, len - len_2);
	ranges[pos].input[0] = CreateRange(off + 0, len_2);
	return pos;
}

void SocialNeedsProcess::ProcessPlatformReactions() {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	
	if (plat_i >= PLATFORM_COUNT) {
		NextPhase();
		return;
	}
	
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	const PlatformAnalysis& pa = MetaDatabase::Single().GetAdd(plat);
	ASSERT(pa.roles.GetCount());
	
	if (sub_batch == 0) {
		ranges.Clear();
		CreateRange(0, pa.roles.GetCount());
		//DUMPC(ranges);
	}
	if (sub_batch >= ranges.GetCount()) {
		NextBatch();
		return;
	}
	int range_i = ranges.GetCount() - 1 - sub_batch;
	
	if (plat_i >= analysis.platforms.GetCount())
		analysis.platforms.SetCount(plat_i+1);
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	if (plat_anal.packed_reactions.GetCount() != ranges.GetCount())
		plat_anal.packed_reactions.Clear();
	plat_anal.packed_reactions.SetCount(ranges.GetCount());
	const String& range_output = plat_anal.packed_reactions[range_i];
	
	if (skip_ready && range_output.GetCount()) {
		NextSubBatch();
		return;
	}
	
	SocialArgs args;
	args.fn = 5;
	Range& range = ranges[range_i];
	if (range.input[0] >= 0) {
		const String& r0 = plat_anal.packed_reactions[range.input[0]];
		args.parts.Add("<merged reaction #1>", r0);
	}
	else {
		int begin = range.off;
		int len_2 = range.len / 2;
		int end = begin + len_2;
		for(int i = begin; i < end; i++) {
			int role_i = pa.roles[i];
			const BiographyRoleAnalysis& role = analysis.roles[role_i];
			args.parts.Add(Capitalize(GetSocietyRoleDescription(role_i)), role.merged_biography_reactions[0]);
		}
	}
	
	if (range.input[1] >= 0) {
		const String& r0 = plat_anal.packed_reactions[range.input[1]];
		args.parts.Add("<merged reaction #2>", r0);
	}
	else {
		int len_2 = range.len / 2;
		int begin = range.off + len_2;
		int end = begin + range.len - len_2;
		for(int i = begin; i < end; i++) {
			int role_i = pa.roles[i];
			const BiographyRoleAnalysis& role = analysis.roles[role_i];
			args.parts.Add(Capitalize(GetSocietyRoleDescription(role_i)), role.merged_biography_reactions[0]);
		}
	}
	
	SetWaiting(1);
	TaskMgr& m = TaskMgr::Single();
	m.GetSocial(args, THISBACK(OnProcessPlatformReactions));
	#endif
}

void SocialNeedsProcess::OnProcessPlatformReactions(String res) {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	int range_i = ranges.GetCount() - 1 - sub_batch;
	if (plat_i >= analysis.platforms.GetCount())
		analysis.platforms.SetCount(plat_i+1);
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	String& s = plat_anal.packed_reactions[range_i];
	
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	NextSubBatch();
	SetWaiting(0);
}

void SocialNeedsProcess::ProcessPlatformDescriptions() {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	
	if (plat_i >= PLATFORM_COUNT) {
		NextPhase();
		return;
	}
	
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	
	if (plat_i >= analysis.platforms.GetCount())
		analysis.platforms.SetCount(plat_i+1);
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	if (skip_ready && plat_anal.profile_description_from_biography.GetCount()) {
		NextBatch();
		return;
	}
	
	if (plat_anal.packed_reactions.IsEmpty()) {
		LOG("error: plat_anal.packed_reactions is empty");
		NextBatch();
		return;
	}
	
	const String& merged_reactions = plat_anal.packed_reactions[0];
	SocialArgs args;
	args.fn = 6;
	args.parts.Add("", merged_reactions);
	
	SetWaiting(1);
	TaskMgr& m = TaskMgr::Single();
	m.GetSocial(args, THISBACK(OnProcessPlatformDescriptions));
	#endif
}

void SocialNeedsProcess::OnProcessPlatformDescriptions(String res) {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	if (plat_i >= analysis.platforms.GetCount())
		analysis.platforms.SetCount(plat_i+1);
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	
	String& s = plat_anal.profile_description_from_biography;
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	NextBatch();
	SetWaiting(0);
}

void SocialNeedsProcess::ProcessPlatformDescriptionRefinements() {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	
	if (plat_i >= PLATFORM_COUNT) {
		NextPhase();
		return;
	}
	
	if (sub_batch >= PLATDESC_LEN_COUNT) {
		NextBatch();
		return;
	}
	int len_i = sub_batch;
	int mode_i = PLATDESC_MODE_FINAL;
	
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	String s = plat_anal.descriptions[len_i][mode_i];
	if (skip_ready && s.GetCount()) {
		NextSubBatch();
		return;
	}
	
	String source;
	if(len_i == 0) {
		source = plat_anal.profile_description_from_biography;
	}
	else {
		source = plat_anal.descriptions[len_i-1][PLATDESC_MODE_FINAL];
	}
	
	const String& merged_reactions = plat_anal.packed_reactions[0];
	SocialArgs args;
	args.fn = len_i == 0 ? 7 : 8;
	args.text = source;
	args.len = GetPlatformDescriptionLength(len_i);
	
	SetWaiting(1);
	TaskMgr& m = TaskMgr::Single();
	m.GetSocial(args, THISBACK(OnProcessPlatformDescriptionRefinements));
	#endif
}

void SocialNeedsProcess::OnProcessPlatformDescriptionRefinements(String res) {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	int len_i = sub_batch;
	int mode_i = PLATDESC_MODE_FINAL;
	String& s = plat_anal.descriptions[len_i][mode_i];
	
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	NextSubBatch();
	SetWaiting(0);
	#endif
}

void SocialNeedsProcess::ProcessPlatformDescriptionTranslated() {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	
	if (plat_i >= PLATFORM_COUNT) {
		NextPhase();
		return;
	}
	int total_subbatches = PLATDESC_MODE_COUNT * PLATDESC_LEN_COUNT;
	if (sub_batch >= total_subbatches) {
		NextBatch();
		return;
	}
	
	int len_i = sub_batch / PLATDESC_MODE_COUNT;
	int mode_i = sub_batch % PLATDESC_MODE_COUNT;
	if (mode_i == PLATDESC_MODE_FINAL) {
		NextSubBatch();
		return;
	}
	
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	String s = plat_anal.descriptions[len_i][mode_i];
	String source = plat_anal.descriptions[len_i][PLATDESC_MODE_FINAL];
	String dst_ln = mode_i == PLATDESC_MODE_FINAL_DIALECT ? "EN-EN" : "FI-FI";
	bool slightly_dialect = (mode_i == PLATDESC_MODE_FINAL_DIALECT || mode_i == PLATDESC_MODE_FINAL_TRANSLATED_DIALECT);
	
	if (skip_ready && s.GetCount()) {
		NextSubBatch();
		return;
	}
	
	const String& merged_reactions = plat_anal.packed_reactions[0];
	
	SetWaiting(1);
	TaskMgr& m = TaskMgr::Single();
	m.Translate("EN-EN", source, dst_ln, THISBACK(OnProcessPlatformDescriptionTranslated), slightly_dialect);
	#endif
}

void SocialNeedsProcess::OnProcessPlatformDescriptionTranslated(String res) {
	BiographyAnalysis& analysis = *this->analysis;
	int plat_i = batch;
	TODO
	#if 0
	const Platform& plat = GetPlatforms()[plat_i];
	PlatformBiographyAnalysis& plat_anal = analysis.platforms[plat_i];
	int len_i = sub_batch / PLATDESC_MODE_COUNT;
	int mode_i = sub_batch % PLATDESC_MODE_COUNT;
	
	String& s = plat_anal.descriptions[len_i][mode_i];
	s = TrimBoth(res);
	if (s.Left(1) == "\"") s = s.Mid(1);
	if (s.Right(1) == "\"") s = s.Left(s.GetCount()-1);
	
	NextSubBatch();
	SetWaiting(0);
	#endif
}



INITIALIZER_COMPONENT(SocialNeeds);
INITIALIZER_COMPONENT_CTRL(SocialNeeds, SocialNeedsCtrl)

END_UPP_NAMESPACE
