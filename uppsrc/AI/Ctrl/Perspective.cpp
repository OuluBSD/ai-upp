#include "Ctrl.h"

NAMESPACE_UPP


PerspectiveCtrl::PerspectiveCtrl() {
	Add(hsplit.VSizePos(0,20).HSizePos());
	
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

void PerspectiveCtrl::Data() {
	TODO
	#if 0
	for(int i = 0; i < mdb.beliefs.GetCount(); i++) {
		Belief& b = mdb.beliefs[i];
		
		beliefs.Set(i, 0, b.name);
	}
	INHIBIT_CURSOR(beliefs);
	
	beliefs.SetCount(mdb.beliefs.GetCount());
	if (!beliefs.IsCursor() && beliefs.GetCount())
		beliefs.SetCursor(0);
	
	DataBelief();
	#endif
}

void PerspectiveCtrl::DataBelief() {
	
	TODO
	#if 0
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
	#endif
}

void PerspectiveCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Paste user data"), TextImgs::BlueRing(), THISBACK1(Do, 2)).Key(K_F5);
}

void PerspectiveCtrl::Do(int fn) {
	DatasetPtrs mp = GetDataset();
	if (!mp.profile || !mp.release)
		return;
	PerspectiveProcess& ss = PerspectiveProcess::Get(*mp.profile, *mp.snap);
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
		TODO
		#if 0
		Belief& b = mdb.beliefs[b_i];
		String res = ReadClipboardText();
		RemoveEmptyLines3(res);
		b.user <<= Split(res, "\n");
		#endif
		DataBelief();
	}
}

void PerspectiveCtrl::OnValueChange() {
	
	if (!beliefs.IsCursor())
		return;
	
	TODO
	#if 0
	int b_i = beliefs.GetCursor();
	Belief& b = mdb.beliefs[b_i];
	
	b.reference = info.reference.GetData();
	#endif
}

void PerspectiveCtrl::AddBelief() {
	
	DatasetPtrs p = GetDataset();
	
	String name;
	bool b = EditTextNotNull(
		name,
		t_("Add Belief"),
		t_("Belief's name"),
		0
	);
	if (!b) return;
	
	TODO
	#if 0
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
	#endif
	Data();
}

void PerspectiveCtrl::RemoveBelief() {
	TODO
	#if 0
	
	DatasetPtrs p = GetDataset();
	if (beliefs.IsCursor()) {
		mdb.beliefs.Remove(beliefs.GetCursor());
	}
	Data();
	#endif
}













PerspectiveProcess::PerspectiveProcess() {
	
}

int PerspectiveProcess::GetPhaseCount() const {
	return PHASE_COUNT;
}

void PerspectiveProcess::DoPhase() {
	TODO
	#if 0
	MetaDatabase& mdb = MetaDatabase::Single();
	if (phase == PHASE_GET_POSITIVE_ATTRS) {
		if (batch >= mdb.beliefs.GetCount()){
			NextPhase();
			return;
		}
		Belief& b = mdb.beliefs[batch];
		if (b.attrs.GetCount()) {
			NextBatch();
			return;
		}
		
		BeliefArgs args;
		args.fn = 0;
		args.user <<= b.user;
		
		SetWaiting(1);
		TaskMgr& m = AiTaskManager();
		m.GetPerspectiveProcess(args, [this,&b](String res) {
			res = TrimBoth(res);
			if (res.Left(2) != "1.")
				res = "1." + res;
			
			RemoveEmptyLines2(res);
			Vector<String> lines = Split(res, "\n");
			
			b.attrs.Clear();
			for(int i = 0; i < lines.GetCount(); i++) {
				b.attrs.Add().positive = lines[i];
			}
			SetWaiting(0);
			NextBatch();
			
			WhenReady();
		});
	}
	else if (phase == PHASE_GET_NEGATIVE_ATTRS) {
		if (batch >= mdb.beliefs.GetCount()){
			NextPhase();
			return;
		}
		Belief& b = mdb.beliefs[batch];
		if (!b.attrs.GetCount() || b.attrs[0].negative.GetCount()) {
			NextBatch();
			return;
		}
		
		BeliefArgs args;
		args.fn = 1;
		for(int i = 0; i < b.attrs.GetCount(); i++)
			args.pos << b.attrs[i].positive;
		
		SetWaiting(1);
		TaskMgr& m = AiTaskManager();
		m.GetPerspectiveProcess(args, [this,&b](String res) {
			res = TrimBoth(res);
			if (res.Left(2) != "1.")
				res = "1." + res;
			
			RemoveEmptyLines2(res);
			Vector<String> lines = Split(res, "\n");
			
			for(int i = 0; i < lines.GetCount(); i++) {
				if (i >= b.attrs.GetCount()) break;
				b.attrs[i].negative = lines[i];
			}
			
			SetWaiting(0);
			NextBatch();
			
			WhenReady();
		});
	}
	#endif
}

PerspectiveProcess& PerspectiveProcess::Get(Profile& e, BiographySnapshot& snap) {
	String t = e.node.GetPath() + ";" + snap.node.GetPath();
	hash_t h = t.GetHashValue();
	static ArrayMap<hash_t, PerspectiveProcess> map;
	int i = map.Find(h);
	if (i >= 0)
		return map[i];
	
	TODO
	#if 0
	PerspectiveProcess& ls = map.Add(h);
	ls.owner = e.owner;
	ls.profile = &e;
	ls.snap = &snap;
	return ls;
	#endif
}







INITIALIZER_COMPONENT(Perspective)
INITIALIZER_COMPONENT_CTRL(Perspective, PerspectiveCtrl)

END_UPP_NAMESPACE
