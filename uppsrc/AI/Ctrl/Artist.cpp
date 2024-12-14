#include "Ctrl.h"

NAMESPACE_UPP

ArtistInfoCtrl::ArtistInfoCtrl() {
	CtrlLayout(*this);
	
	for(int i = 0; i < GENDER_COUNT; i++)
		gender.Add(GetGenderString(i));
	
	native_name <<= THISBACK(OnValueChange);
	english_name <<= THISBACK(OnValueChange);
	year_of_birth <<= THISBACK(OnValueChange);
	year_of_career_begin <<= THISBACK(OnValueChange);
	biography <<= THISBACK(OnValueChange);
	text_style <<= THISBACK(OnValueChange);
	vibe_of_text <<= THISBACK(OnValueChange);
	natural_tools <<= THISBACK(OnValueChange);
	electronic_tools <<= THISBACK(OnValueChange);
	speaker_visually <<= THISBACK(OnValueChange);
	gender <<= THISBACK(OnValueChange);
	language <<= THISBACK(OnValueChange);
	
}

void ArtistInfoCtrl::Clear() {
	this->native_name				.Clear();
	this->english_name				.Clear();
	this->year_of_birth				.Clear();
	this->year_of_career_begin		.Clear();
	this->biography					.Clear();
	this->text_style				.Clear();
	this->vibe_of_text				.Clear();
	this->natural_tools				.Clear();
	this->electronic_tools			.Clear();
	this->speaker_visually			.Clear();
	this->gender					.SetIndex(0);
}

void ArtistInfoCtrl::Data() {
	DatasetPtrs p = GetDataset();
	
	if (language.GetCount() == 0 && GetLanguageCount()) {
		for(int i = 0; i < GetLanguageCount(); i++)
			language.Add(GetLanguageKey(i));
		language.SetIndex(0);
	}
	
	Clear();
	
	if (p.entity) {
		Entity& a = *p.entity;
			
		this->native_name				.SetData(a.Data("native_name"));
		this->english_name				.SetData(a.Data("english_name"));
		this->year_of_birth				.SetData(a.Data("year_of_birth"));
		this->year_of_career_begin		.SetData(a.Data("year_of_career_begin"));
		this->biography					.SetData(a.Data("biography"));
		this->text_style				.SetData(a.Data("text_style"));
		this->vibe_of_text				.SetData(a.Data("vibe_of_text"));
		this->natural_tools				.SetData(a.Data("natural_tools"));
		this->electronic_tools			.SetData(a.Data("electronic_tools"));
		this->speaker_visually			.SetData(a.Data("speaker_visually"));
		//this->gender						.SetIndex();
		this->language					.SetIndex(a.Data("language"));
	}
	
	
}

void ArtistInfoCtrl::OnValueChange() {
	DatasetPtrs p = GetDataset();
	
	if (p.entity) {
		Entity& o = *p.entity;
		o.Data("native_name")				= this->native_name.GetData();
		o.Data("english_name")				= this->english_name.GetData();
		o.Data("year_of_birth")				= this->year_of_birth.GetData();
		o.Data("year_of_career_begin")		= this->year_of_career_begin.GetData();
		o.Data("biography")					= this->biography.GetData();
		o.Data("text_style")				= this->text_style.GetData();
		o.Data("vibe_of_text")				= this->vibe_of_text.GetData();
		o.Data("natural_tools")				= this->natural_tools.GetData();
		o.Data("electronic_tools")			= this->electronic_tools.GetData();
		o.Data("speaker_visually")			= this->speaker_visually.GetData();
		o.Data("gender")					= this->gender.GetIndex();
		o.Data("language")					= this->language.GetIndex();
		
		//int c = editor->entities.GetCursor();
		//editor->entities.Set(c, 0, o.native_name);
	}
}

INITIALIZER_COMPONENT(Artist);
INITIALIZER_COMPONENT_CTRL(Artist, ArtistInfoCtrl)


END_UPP_NAMESPACE
