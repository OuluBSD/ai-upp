#include "Ctrl.h"

NAMESPACE_UPP


ProfileInfoCtrl::ProfileInfoCtrl() {
	CtrlLayout(*this);
	
	name <<= THISBACK(OnValueChange);
	begin <<= THISBACK(OnValueChange);
	biography <<= THISBACK(OnValueChange);
	preferred_genres <<= THISBACK(OnValueChange);
	
	languages.AddColumn(t_("Language"));
	languages.AddColumn(t_("Set"));
	languages.ColumnWidths("4 1");
	languages.SetCount(LNG_COUNT);
	for(int i = 0; i < LNG_COUNT; i++) {
		languages.Set(i, 0, Capitalize(GetLanguageKey(i)));
		Option* o = new Option;
		o->WhenAction << THISBACK(OnValueChange);
		languages.SetCtrl(i, 1, o);
	}
	languages <<= THISBACK(OnValueChange);
}

void ProfileInfoCtrl::Data() {
	
	DatasetPtrs p = GetDataset();
	
	Clear();
	
	if (p.profile) {
		Profile& a = *p.profile;
			
		this->name						.SetData(a.name);
		this->begin						.SetData(a.begin);
		this->biography					.SetData(a.biography);
		this->preferred_genres			.SetData(a.preferred_genres);
		
		for(int i = 0; i < LNG_COUNT; i++) {
			Ctrl* c = languages.GetCtrl(i, 1);
			if (!c) continue;
			Option* opt = dynamic_cast<Option*>(c);
			ASSERT(opt);
			opt->Set(a.languages.Find(i) >= 0);
		}
	}
	
}

void ProfileInfoCtrl::Clear() {
	this->name						.Clear();
	this->begin						.Clear();
	this->biography					.Clear();
	this->preferred_genres			.Clear();
	for(int i = 0; i < LNG_COUNT; i++) {
		Ctrl* c = languages.GetCtrl(i, 1);
		Option* opt = dynamic_cast<Option*>(c);
		if (opt) opt->Set(0);
	}
		
}

void ProfileInfoCtrl::OnValueChange() {
	DatasetPtrs p = GetDataset();
	
	TODO
	#if 0
	if (p.profile && p.leads->profiles.IsCursor()) {
		Profile& o = *p.profile;
		o.name						= this->name.GetData();
		o.begin						= this->begin.GetData();
		o.biography					= this->biography.GetData();
		o.preferred_genres			= this->preferred_genres.GetData();
		
		
		o.languages.Clear();
		for(int i = 0; i < LNG_COUNT; i++) {
			Option* opt = dynamic_cast<Option*>(languages.GetCtrl(i, 1));
			ASSERT(opt);
			if (opt->Get())
				o.languages.Add(i);
		}
		
		int c = p.leads->profiles.GetCursor();
		p.leads->profiles.Set(c, 0, o.name);
	}
	#endif
}

INITIALIZER_COMPONENT_CTRL(Profile, ProfileInfoCtrl)


END_UPP_NAMESPACE
