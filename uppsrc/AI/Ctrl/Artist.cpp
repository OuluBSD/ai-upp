#include "Ctrl.h"

NAMESPACE_UPP

ArtistInfoCtrl::ArtistInfoCtrl() {
	CtrlLayout(*this);
	
	sex.Add(t_("Male"));
	sex.Add(t_("Female"));
	
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
	sex <<= THISBACK(OnValueChange);
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
	this->sex						.SetIndex(0);
}

void ArtistInfoCtrl::Data() {
	TextDatabase& db = GetDatabase();
	EditorPtrs& p = GetPointers();
	
	lbl_entity.SetLabel(GetAppModeKeyCap(AM_ENTITY));
	lbl_speaker.SetLabel(GetAppModeLabel(AML_SPEAKER));
	lbl_text_style.SetLabel(GetAppModeLabel(AML_TALKINGSTYLE));
	lbl_natural_tools.SetLabel(GetAppModeLabel(AML_NATURAL_TOOLS));
	lbl_electronic_tools.SetLabel(GetAppModeLabel(AML_ELECTRONIC_TOOLS));
	lbl_vibe_of_text.SetLabel(GetAppModeLabel(AML_VIBE_OF_TEXT));
	
	if (language.GetCount() == 0 && GetLanguageCount()) {
		for(int i = 0; i < GetLanguageCount(); i++)
			language.Add(GetLanguageKey(i));
		language.SetIndex(0);
	}
	
	Clear();
	
	if (p.entity) {
		Entity& a = *p.entity;
			
		this->native_name				.SetData(a.native_name);
		this->english_name				.SetData(a.english_name);
		this->year_of_birth				.SetData(a.year_of_birth);
		this->year_of_career_begin		.SetData(a.year_of_career_begin);
		this->biography					.SetData(a.biography);
		this->text_style				.SetData(a.text_style);
		this->vibe_of_text				.SetData(a.vibe_of_text);
		this->natural_tools				.SetData(a.natural_tools);
		this->electronic_tools			.SetData(a.electronic_tools);
		this->speaker_visually			.SetData(a.speaker_visually);
		this->sex						.SetIndex(a.is_female);
		this->language					.SetIndex(a.language);
	}
	
	
}

void ArtistInfoCtrl::OnValueChange() {
	TextDatabase& db = GetDatabase();
	EditorPtrs& p = GetPointers();
	MetaPtrs& mp = MetaPtrs::Single();
	
	if (p.entity && p.editor->profiles.IsCursor()) {
		Entity& o = *p.entity;
		ASSERT(o.profile == mp.profile);
		o.native_name				= this->native_name.GetData();
		o.english_name				= this->english_name.GetData();
		o.year_of_birth				= this->year_of_birth.GetData();
		o.year_of_career_begin		= this->year_of_career_begin.GetData();
		o.biography					= this->biography.GetData();
		o.text_style				= this->text_style.GetData();
		o.vibe_of_text				= this->vibe_of_text.GetData();
		o.natural_tools				= this->natural_tools.GetData();
		o.electronic_tools			= this->electronic_tools.GetData();
		o.speaker_visually			= this->speaker_visually.GetData();
		o.is_female					= this->sex.GetIndex();
		o.language					= this->language.GetIndex();
		
		//int c = editor->entities.GetCursor();
		//editor->entities.Set(c, 0, o.native_name);
	}
}

END_UPP_NAMESPACE
