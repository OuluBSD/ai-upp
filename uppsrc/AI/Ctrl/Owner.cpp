#include "Ctrl.h"

NAMESPACE_UPP


OwnerInfoCtrl::OwnerInfoCtrl() {
	CtrlLayout(*this);
	
	name <<= THISBACK(OnValueChange);
	year_of_birth <<= THISBACK(OnValueChange);
	year_of_hobbyist_begin <<= THISBACK(OnValueChange);
	year_of_career_begin <<= THISBACK(OnValueChange);
	biography <<= THISBACK(OnValueChange);
	is_guitarist <<= THISBACK(OnValueChange);
	electronic_tools <<= THISBACK(OnValueChange);
	
}

void OwnerInfoCtrl::Data() {
	
	DatasetPtrs p = GetDataset();
	
	Clear();
	
	if (p.owner) {
		Owner& a = *p.owner;
			
		this->name						.SetData(a.name);
		this->year_of_birth				.SetData(a.year_of_birth);
		this->year_of_hobbyist_begin	.SetData(a.year_of_hobbyist_begin);
		this->year_of_career_begin		.SetData(a.year_of_career_begin);
		this->biography					.SetData(a.biography);
		this->is_guitarist				.SetData(a.is_guitarist);
		this->electronic_tools			.SetData(a.electronic_tools);
	}
	
}

void OwnerInfoCtrl::Clear() {
	this->name						.Clear();
	this->year_of_birth				.Clear();
	this->year_of_hobbyist_begin	.Clear();
	this->year_of_career_begin		.Clear();
	this->biography					.Clear();
	this->is_guitarist				.Set(0);
	this->electronic_tools			.Clear();
}

void OwnerInfoCtrl::OnValueChange() {
	DatasetPtrs p = GetDataset();
	
	TODO
	#if 0
	if (p.owner && p.leads->profiles.IsCursor()) {
		Owner& o = *p.owner;
		o.name						= this->name.GetData();
		o.year_of_birth				= this->year_of_birth.GetData();
		o.year_of_hobbyist_begin	= this->year_of_hobbyist_begin.GetData();
		o.year_of_career_begin		= this->year_of_career_begin.GetData();
		o.biography					= this->biography.GetData();
		o.is_guitarist				= this->is_guitarist.Get();
		o.electronic_tools			= this->electronic_tools.GetData();
		
		int c = p.leads->profiles.GetCursor();
		p.leads->profiles.Set(c, 0, o.name);
	}
	#endif
}

INITIALIZER_COMPONENT_CTRL(Owner, OwnerInfoCtrl)


END_UPP_NAMESPACE
