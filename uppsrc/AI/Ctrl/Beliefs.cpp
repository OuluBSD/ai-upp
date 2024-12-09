#include "Ctrl.h"

NAMESPACE_UPP


SocialBeliefsCtrl::SocialBeliefsCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	Add(prog.BottomPos(0,20).HSizePos(300));
	Add(remaining.BottomPos(0,20).LeftPos(0,300));
	
	hsplit.Add(beliefs);
	hsplit.Add(info);
	hsplit.Add(user);
	hsplit.Add(attrs);
	hsplit.SetPos(1500,0);
	hsplit.SetPos(1500+8500/3,1);
	hsplit.SetPos(1500+8500*2/3,2);
	
	CtrlLayout(info);
	info.reference.WhenAction << THISBACK(OnValueChange);
	
	beliefs.AddColumn("Belief");
	beliefs.WhenCursor << THISBACK(DataBelief);
	beliefs.WhenBar = [this](Bar& b) {
		b.Add("Add belief", THISBACK(AddBelief));
		//b.Add("Remove belief", THISBACK(RemoveBelief));
	};
	
	user.AddColumn("User input");
	
	attrs.AddColumn("Positive");
	attrs.AddColumn("Negative");
	
	
}

void SocialBeliefsCtrl::Data() {
	
	
	
	for(int i = 0; i < mdb.beliefs.GetCount(); i++) {
		Belief& b = mdb.beliefs[i];
		
		beliefs.Set(i, 0, b.name);
	}
	INHIBIT_CURSOR(beliefs);
	beliefs.SetCount(mdb.beliefs.GetCount());
	if (!beliefs.IsCursor() && beliefs.GetCount())
		beliefs.SetCursor(0);
	
	DataBelief();
}

void SocialBeliefsCtrl::DataBelief() {
	
	if (!beliefs.IsCursor())
		return;
	
	int b_i = beliefs.GetCursor();
	Belief& b = mdb.beliefs[b_i];
	
	
	info.name.SetData(b.name);
	info.reference.SetData(b.reference);
	
	for(int i = 0; i < b.user.GetCount(); i++) {
		user.Set(i, 0, b.user[i]);
	}
	user.SetCount(b.user.GetCount());
	
	for(int i = 0; i < b.attrs.GetCount(); i++) {
		Belief::Attr& a = b.attrs[i];
		attrs.Set(i, 0, a.positive);
		attrs.Set(i, 1, a.negative);
	}
	attrs.SetCount(b.attrs.GetCount());
	
}

void SocialBeliefsCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), AppImg::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), AppImg::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Paste user data"), AppImg::BlueRing(), THISBACK1(Do, 2)).Key(K_F5);
}

void SocialBeliefsCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.snap)
		return;
	SocialBeliefsProcess& ss = SocialBeliefsProcess::Get(*mp.profile, *mp.snap);
	if (fn == 0) {
		ss.Start();
	}
	else if (fn == 1) {
		ss.Stop();
	}
	else if (fn == 2) {
		
		if (!beliefs.IsCursor())
			return;
		int b_i = beliefs.GetCursor();
		Belief& b = mdb.beliefs[b_i];
		String res = ReadClipboardText();
		RemoveEmptyLines3(res);
		b.user <<= Split(res, "\n");
		DataBelief();
	}
}

void SocialBeliefsCtrl::OnValueChange() {
	
	if (!beliefs.IsCursor())
		return;
	
	int b_i = beliefs.GetCursor();
	Belief& b = mdb.beliefs[b_i];
	
	b.reference = info.reference.GetData();
}

void SocialBeliefsCtrl::AddBelief() {
	
	DatasetPtrs p = GetDataset();
	
	String name;
	bool b = EditTextNotNull(
		name,
		t_("Add Belief"),
		t_("Belief's name"),
		0
	);
	if (!b) return;
	
	String t = MakeTitle(name);
	int artist_i = -1;
	for(int i = 0; i < mdb.beliefs.GetCount(); i++) {
		Belief& a = mdb.beliefs[i];
		if (a.name == t) {
			artist_i = i;
			break;
		}
	}
	if (artist_i >= 0) {
		PromptOK(DeQtf(t_("Belief exist already")));
		return;
	}
	
	Belief& a = mdb.beliefs.Add();
	a.uniq = Random64();
	a.name = name;
	
	Data();
}

void SocialBeliefsCtrl::RemoveBelief() {
	
	DatasetPtrs p = GetDataset();
	if (beliefs.IsCursor()) {
		mdb.beliefs.Remove(beliefs.GetCursor());
	}
	Data();
}


END_UPP_NAMESPACE
