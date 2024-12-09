#include "Ctrl.h"

NAMESPACE_UPP


AudienceCtrl::AudienceCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	hsplit.Horz() << menusplit << vsplit;
	hsplit.SetPos(1500);
	
	menusplit.Vert() << roles << profiles;
	menusplit.SetPos(3333);
	
	vsplit.Vert() << responses << bsplit;
	vsplit.SetPos(3333);
	
	bsplit.Horz() << entry << img;
	
	CtrlLayout(entry);
	
	roles.AddColumn(t_("Enabled"));
	roles.AddColumn(t_("Role"));
	roles.ColumnWidths("1 9");
	Color clr[2] = {Color(255, 216, 246), Color(199, 201, 255)};
	
	for(int i = 0; i < SOCIETYROLE_COUNT; i++) {
		int sex = i % 2;
		const Color& c = clr[sex];
		String s = GetSocietyRoleKey(i);
		s.Replace("Representative Of The Organization", "Rep.");
		roles.Set(i, 1, AttrText(s)
			.NormalPaper(c).NormalInk(Black())
			.Paper(Blend(c, Black())).Ink(White()));
	}
	roles.WhenCursor << THISBACK(DataRole);
	
	profiles.AddColumn(t_("Profile"));
	profiles.WhenCursor << THISBACK(DataProfile);
	
	responses.AddColumn(t_("Year"));
	responses.AddColumn(t_("Age"));
	responses.AddColumn(t_("Text"));
	responses.AddColumn(t_("Keyword"));
	responses.AddIndex("IDX");
	responses.ColumnWidths("1 1 10 3");
	responses.WhenCursor << THISBACK(DataResponse);
	
	
}

void AudienceCtrl::Data() {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.biography) return;
	INHIBIT_CURSOR(roles);
	if (!roles.IsCursor()) roles.SetCursor(0);
	
	
	// Check if role is enabled (indirectly by enabled platforms)
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	BiographyAnalysis& analysis = *mp.analysis;
	analysis.Realize();
	Index<int> req_roles = analysis.GetRequiredRoles();
	for(int role_i = 0; role_i < SOCIETYROLE_COUNT; role_i++) {
		bool enabled = req_roles.Find(role_i) >= 0;
		roles.Set(role_i, 0, enabled ? "X":"");
	}
	
	DataRole();
}

void AudienceCtrl::DataRole() {
	if (!roles.IsCursor())
		return;
	
	int role_i = roles.GetCursor();
	const Array<RoleProfile>& profs = GetRoleProfile(role_i);
	for(int i = 0; i < profs.GetCount(); i++) {
		profiles.Set(i, 0, profs[i].name);
	}
	profiles.SetCount(profs.GetCount());
	INHIBIT_CURSOR(profiles);
	if (profiles.GetCount() && !profiles.IsCursor()) profiles.SetCursor(0);
	
	DataProfile();
}

void AudienceCtrl::DataProfile() {
	
	DatasetPtrs mp = GetDataset();
	
	if (!roles.IsCursor() || !profiles.IsCursor())
		return;
	int role_i = roles.GetCursor();
	int prof_i = profiles.GetCursor();
	
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	BiographyAnalysis& analysis = *mp.analysis;
	analysis.Realize();
	const BiographyProfileAnalysis& pa = analysis.profiles[role_i][prof_i];
	const Array<RoleProfile>& profs = GetRoleProfile(role_i);
	const RoleProfile& prof = profs[prof_i];
	
	String cat_str;
	for(int i = 0; i < pa.categories.GetCount(); i++) {
		int cat_i = pa.categories.GetKey(i);
		cat_str += IntStr(i+1) + ". " + GetBiographyCategoryKey(cat_i) + ": " + pa.categories[i] + "\n";
	}
	entry.categories.SetData(cat_str);
	entry.description.SetData(prof.profile);
	
	for(int i = 0; i < pa.responses.GetCount(); i++) {
		const BiographyProfileAnalysis::Response& resp = pa.responses[i];
		responses.Set(i, "IDX", i);
		responses.Set(i, 0, resp.year);
		responses.Set(i, 1, resp.category);
		responses.Set(i, 2, resp.text);
		responses.Set(i, 3, resp.keywords);
	}
	responses.SetCount(pa.responses.GetCount());
	INHIBIT_CURSOR(responses);
	if (responses.GetCount() && !responses.IsCursor()) responses.SetCursor(0);
	
	DataResponse();
}

void AudienceCtrl::DataResponse() {
	
	DatasetPtrs mp = GetDataset();
	
	if (!roles.IsCursor() || !profiles.IsCursor() || !responses.IsCursor())
		return;
	int role_i = roles.GetCursor();
	int prof_i = profiles.GetCursor();
	int resp_i = responses.GetCursor();
	if (!responses.IsCursor())
		return;
	
	
	Owner& owner = *mp.owner;
	Biography& biography = *mp.biography;
	BiographyAnalysis& analysis = *mp.analysis;
	const BiographyProfileAnalysis& pa = analysis.profiles[role_i][prof_i];
	const BiographyProfileAnalysis::Response& resp = pa.responses[resp_i];
	const Array<RoleProfile>& profs = GetRoleProfile(role_i);
	const RoleProfile& prof = profs[prof_i];
	
	entry.text.SetData(resp.text);
	entry.keywords.SetData(resp.keywords);
	entry.empathy_score.SetData(resp.score[BIOSCORE_EMPATHY]);
	entry.attraction_score.SetData(resp.score[BIOSCORE_ATTRACTION]);
	entry.value_score.SetData(resp.score[BIOSCORE_VALUE]);
	entry.leadership_score.SetData(resp.score[BIOSCORE_LEADERSHIP]);
	
}
/*
void AudienceCtrl::MakeKeywords(int fn) {
	TaskMgr& m = TaskMgr::Single();
	SocialArgs args;
	if (fn == 0)
		args.text = year.text.GetData();
	else
		args.text = year.image_text.GetData();
	m.GetSocial(args, [this,fn](String s) {PostCallback(THISBACK2(OnKeywords, fn, s));});
}

void AudienceCtrl::Translate() {
	TaskMgr& m = TaskMgr::Single();
	
	String src = year.native_text.GetData();
	
	m.Translate("FI-FI", src, "EN-US", [this](String s) {PostCallback(THISBACK1(OnTranslate, s));});
}

void AudienceCtrl::OnTranslate(String s) {
	year.text.SetData(s);
	OnValueChange();
}

void AudienceCtrl::OnKeywords(int fn, String s) {
	RemoveEmptyLines2(s);
	Vector<String> parts = Split(s, "\n");
	s = Join(parts, ", ");
	if (fn == 0)
		year.keywords.SetData(s);
	else
		year.image_keywords.SetData(s);
	OnValueChange();
}
*/
void AudienceCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), AppImg::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	
	/*
	bar.Add(t_("Translate"), AppImg::BlueRing(), THISBACK(Translate)).Key(K_F5);
	bar.Add(t_("Make keywords"), AppImg::BlueRing(), THISBACK1(MakeKeywords, 0)).Key(K_F6);
	bar.Add(t_("Make keywords (image)"), AppImg::BlueRing(), THISBACK1(MakeKeywords, 1)).Key(K_F7);
	*/
}

void AudienceCtrl::EntryListMenu(Bar& bar) {
	
}

void AudienceCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.snap)
		return;
	AudienceProcess& ss = AudienceProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
}


END_UPP_NAMESPACE
