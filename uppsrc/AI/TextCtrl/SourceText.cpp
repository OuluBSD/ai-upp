#include "TextCtrl.h"

NAMESPACE_UPP

SourceDataCtrl::SourceDataCtrl(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << vsplit << scripts << analysis;
	hsplit.SetPos(2500);
	
	vsplit.Vert() << entities << components;
	vsplit.SetPos(1000,0);
	vsplit.SetPos(5500,1);
	
	entities.AddColumn(t_("File"));
	entities.WhenCursor << THISBACK(DataEntity);
	
	components.AddColumn(t_("Entry"));
	components.WhenCursor << THISBACK(DataExtension);
	
}

void SourceDataCtrl::SetFont(Font fnt) {
	scripts.SetFont(fnt);
	analysis.SetFont(fnt);
}

void SourceDataCtrl::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		entities.Clear();
		components.Clear();
		analysis.Clear();
		return;
	}
	auto& src = p.src->Data();
	const auto& data = src.entities;
	
	//DUMP(GetDatabase().a.dataset.scripts.GetCount());
	
	entities.SetCount(data.GetCount());
	for(int i = 0; i < data.GetCount(); i++) {
		const auto& ea = data[i];
		String s = ea.name;
		if (GetDefaultCharset() != CHARSET_UTF8)
			s = ToCharset(CHARSET_DEFAULT, s, CHARSET_UTF8);
		
		entities.Set(i, 0, s);
		entities.Set(i, 1, Join(ea.genres, ", "));
	}
	
	if (!entities.IsCursor() && entities.GetCount())
		entities.SetCursor(0);
	
	/*if (0) {
		int scripts_total = 0;
		for (const auto& d : data)
			scripts_total += d.scripts.GetCount();
		DUMP(scripts_total);
	}*/
	DataEntity();
}

void SourceDataCtrl::DataEntity() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		components.Clear();
		analysis.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	if (!entities.IsCursor()) return;
	int acur = entities.GetCursor();
	const auto& data = src.entities;
	const auto& artist = data[acur];
	
	components.SetCount(artist.scripts.GetCount());
	for(int i = 0; i < artist.scripts.GetCount(); i++) {
		String s = artist.scripts[i].name;
		if (GetDefaultCharset() != CHARSET_UTF8)
			s = ToCharset(CHARSET_DEFAULT, s, CHARSET_UTF8);
		
		components.Set(i, 0, s);
	}
	
	if (!components.IsCursor() && components.GetCount())
		components.SetCursor(0);
	
	DataExtension();
}

void SourceDataCtrl::DataExtension() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		analysis.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	if (!entities.IsCursor() || !components.IsCursor()) return;
	int acur = entities.GetCursor();
	int scur = components.GetCursor();
	const auto& data = src.entities;
	const auto& artist = data[acur];
	const auto& song = artist.scripts[scur];
	
	if (data_type == 0) {
		String s = song.text;
		if (GetDefaultCharset() != CHARSET_UTF8)
			s = ToCharset(CHARSET_DEFAULT, s, CHARSET_UTF8);
		this->scripts.SetData(s);
		analysis.Clear();
		
		TryNo5tStructureSolver solver;
		solver.Process(s);
		analysis.SetData(solver.GetResult());
		//analysis.SetData(solver.GetDebugLines());
	}
	else if (data_type == 1) {
		String key = artist.name + " - " + song.name;
		int ss_i = src.scripts.Find(key.GetHashValue());
		if (ss_i < 0) {
			scripts.Clear();
			analysis.Clear();
			return;
		}
		
		ScriptStruct& ss = src.scripts[ss_i];
		String txt = src.GetScriptDump(ss_i);
		scripts.SetData(txt);
		analysis.Clear();
	}
}

















TokensPage::TokensPage(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << tokens;
	hsplit.SetPos(2000);
	
	tokens.AddColumn(t_("Token"));
	tokens.AddColumn(t_("Count"));
	tokens.AddIndex("IDX");
	tokens.ColumnWidths("3 1");
	tokens.WhenBar << [this](Bar& bar){
		bar.Add("Copy", [this]() {
			int i = tokens.GetCursor();
			String text = tokens.Get(i, 0);
			WriteClipboardText(text);
		});
	};
}

void TokensPage::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		tokens.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	for(int j = 0; j < src.tokens.GetCount(); j++) {
		const String& txt = src.tokens.GetKey(j);
		const Token& tk = src.tokens[j];
		
		tokens.Set(j, 0, txt);
	}
	tokens.SetCount(src.tokens.GetCount());
	
}












AmbiguousWordPairs::AmbiguousWordPairs(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << texts;
	hsplit.SetPos(2000);
	
	texts.AddColumn(t_("From"));
	texts.AddColumn(t_("To"));
	texts.AddColumn(t_("From Type"));
	texts.AddColumn(t_("To Type"));
	texts.AddColumn();
	texts.AddIndex("IDX");
	texts.ColumnWidths("1 1 1 1 6");
	texts.WhenBar << [this](Bar& bar){
		bar.Add("Copy", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 0);
			WriteClipboardText(text);
		});
		bar.Add("Copy virtual text", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 1);
			WriteClipboardText(text);
		});
	};
	
}

void AmbiguousWordPairs::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		texts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	int row = 0;
	for(int i = 0; i < src.ambiguous_word_pairs.GetCount(); i++) {
		const WordPairType& wp = src.ambiguous_word_pairs[i];
		if (wp.from >= 0 && wp.to >= 0) {
			const String& from = src.words.GetKey(wp.from);
			const String& to = src.words.GetKey(wp.to);
			
			texts.Set(row, 0, from);
			texts.Set(row, 1, to);
			texts.Set(row, 2, wp.from_type >= 0 ? src.word_classes[wp.from_type] : String());
			texts.Set(row, 3, wp.to_type >= 0 ? src.word_classes[wp.to_type] : String());
			
			row++;
		}
	}
	texts.SetCount(row);
	
}














VirtualPhrases::VirtualPhrases(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << vsplit;
	hsplit.SetPos(2000);
	
	vsplit.Vert() << texts << parts;
	
	texts.AddColumn(t_("Phrase"));
	texts.AddColumn(t_("Classes"));
	texts.AddColumn(t_("Parts"));
	texts.AddColumn(t_("Category"));
	texts.AddIndex("IDX");
	texts.ColumnWidths("1 1 1 1");
	texts.WhenBar << [this](Bar& bar){
		bar.Add("Copy", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 0);
			WriteClipboardText(text);
		});
		bar.Add("Copy virtual text", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 1);
			WriteClipboardText(text);
		});
	};
	
	parts.AddColumn(t_("From"));
	parts.AddIndex("IDX");
	parts.WhenBar << [this](Bar& bar){
		bar.Add("Copy virtual text", [this]() {
			int i = parts.GetCursor();
			String text = parts.Get(i, 0);
			WriteClipboardText(text);
		});
	};
	
}

String GetTypePhraseString(const Vector<int>& word_classes, const SrcTextData& src) {
	String o;
	for (int wc_i : word_classes) {
		if (wc_i >= src.word_classes.GetCount()) {
			o << "{error}";
			continue;
		}
		String wc = src.word_classes[wc_i];
		
		int a = wc.Find(",");
		if (a >= 0)
			wc = TrimBoth(wc.Left(a));
		
		a = wc.Find("(");
		if (a >= 0)
			wc = TrimBoth(wc.Left(a));
		
		o << "{" << wc << "}";
	}
	return o;
}

void VirtualPhrases::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		texts.Clear();
		return;
	}
	auto& src = p.src->Data();
	int row = 0;
	for(int i = 0; i < src.token_texts.GetCount(); i++) {
		const TokenText& txt = src.token_texts[i];
		if (txt.virtual_phrase < 0)
			continue;
		
		VirtualPhrase& vp = src.virtual_phrases[txt.virtual_phrase];
		
		String txt_str = src.GetTokenTextString(txt);
		String type_str = GetTypePhraseString(vp.word_classes, src);
		String parts_str;
		String struct_str;
		if (vp.virtual_phrase_struct >= 0) {
			VirtualPhraseStruct& vps = src.virtual_phrase_structs[vp.virtual_phrase_struct];
			int struct_type = vps.struct_type;
			if (struct_type >= 0)
				struct_str = src.struct_types[struct_type];
			for (int vpp_i : vps.virtual_phrase_parts) {
				if (!parts_str.IsEmpty()) parts_str += " + ";
				const VirtualPhrasePart& vpp = src.virtual_phrase_parts[vpp_i];
				parts_str += vpp.struct_part_type >= 0 ? src.struct_part_types[vpp.struct_part_type] : String("error");
			}
		}
		texts.Set(row, 0, txt_str);
		texts.Set(row, 1, type_str);
		texts.Set(row, 2, parts_str);
		texts.Set(row, 3, struct_str);
		row++;
		
		if (row >= 10000)
			break;
	}
	texts.SetCount(row);
	
	row = 0;
	for(int i = 0; i < src.virtual_phrase_parts.GetCount(); i++) {
		const VirtualPhrasePart& vpp = src.virtual_phrase_parts[i];
		if (vpp.word_classes.IsEmpty())
			continue;
		
		String type_str = GetTypePhraseString(vpp.word_classes, src);
		parts.Set(row, 0, type_str);
		row++;
	}
	parts.SetCount(row);
	
	
}












VirtualPhraseParts::VirtualPhraseParts(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << texts;
	hsplit.SetPos(2000);
	
	//vsplit.Vert() << texts << parts;
	
	texts.AddColumn(t_("Types"));
	texts.AddColumn(t_("Structural name"));
	texts.AddColumn(t_("Count"));
	texts.AddIndex("IDX");
	texts.ColumnWidths("3 2 1");
	texts.WhenBar << [this](Bar& bar){
		bar.Add("Copy virtual type", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 0);
			WriteClipboardText(text);
		});
		bar.Add("Copy type", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 1);
			WriteClipboardText(text);
		});
	};
	
}

void VirtualPhraseParts::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		texts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	int row = 0;
	for(int i = 0; i < src.virtual_phrase_parts.GetCount(); i++) {
		const VirtualPhrasePart& vpp = src.virtual_phrase_parts[i];
		String type_name;
		if (vpp.struct_part_type >= 0) {
			type_name = src.struct_part_types[vpp.struct_part_type];
		}
		
		String type_str = GetTypePhraseString(vpp.word_classes, src);
		texts.Set(row, 0, type_str);
		texts.Set(row, 1, type_name);
		texts.Set(row, 2, vpp.count);
		texts.Set(row, "IDX", i);
		row++;
		
		if (row >= 10000) break;
	}
	texts.SetCount(row);
	texts.SetSortColumn(2, true);
	
}







VirtualPhraseStructs::VirtualPhraseStructs(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << texts;
	hsplit.SetPos(2000);
	
	//vsplit.Vert() << texts << parts;
	
	texts.AddColumn(t_("Structural name"));
	texts.AddColumn(t_("Types"));
	texts.AddColumn("");
	texts.AddIndex("IDX");
	texts.ColumnWidths("2 3 3");
	texts.WhenBar << [this](Bar& bar){
		bar.Add("Copy virtual type", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 0);
			WriteClipboardText(text);
		});
		bar.Add("Copy type", [this]() {
			int i = texts.GetCursor();
			String text = texts.Get(i, 1);
			WriteClipboardText(text);
		});
	};
	
}

void VirtualPhraseStructs::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		texts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	int row = 0;
	for(int i = 0; i < src.virtual_phrase_structs.GetCount(); i++) {
		const VirtualPhraseStruct& vps = src.virtual_phrase_structs[i];
		
		String type_name;
		if (vps.struct_type >= 0) {
			type_name = src.struct_types[vps.struct_type];
		}
		
		String type_str;
		for(int j = 0; j < vps.virtual_phrase_parts.GetCount(); j++) {
			if (j) type_str += " + ";
			int vpp_i = vps.virtual_phrase_parts[j];
			VirtualPhrasePart& vpp = src.virtual_phrase_parts[vpp_i];
			if (vpp.struct_part_type < 0) {
				type_str += "error";
			}
			else {
				const String& part_type_name = src.struct_part_types[vpp.struct_part_type];
				type_str += part_type_name;
			}
		}
		texts.Set(row, 0, type_name);
		texts.Set(row, 1, type_str);
		texts.Set(row, "IDX", i);
		row++;
		
		if (row >= 10000) break;
	}
	texts.SetCount(row);
	texts.SetSortColumn(2, true);
	
}









void ScoreDisplay::Paint(Draw& d, const Rect& r, const Value& q,
                         Color ink, Color paper, dword style) const {
	d.DrawRect(r, Blend(paper, White()));
	ValueArray va = q;
	if (va.GetCount() == SCORE_COUNT) {
		int w = r.Width();
		int h = r.Height();
		double cx = (double)w / (double)SCORE_COUNT;
		Color clr = LtBlue();
		for(int i = 0; i < va.GetCount(); i++) {
			int sc = va[i];
			int h0 = h * sc / 10;
			int x0 = (int)(i * cx);
			int x1 = (int)((i+1) * cx);
			int y0 = h - h0;
			d.DrawRect(Rect(x0,y0,x1,h), clr);
		}
		for(int i = 1; i < SCORE_COUNT; i++) {
			int x0 = (int)(i * cx);
			d.DrawLine(x0,0,x0,h,1,GrayColor());
		}
	}
}



PhrasePartAnalysis::PhrasePartAnalysis(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());

	hsplit.Horz() << vsplit << parts;
	hsplit.SetPos(2000);

	vsplit.Vert() << attrs << colors << actions << action_args;

	attrs.AddColumn(t_("Group"));
	attrs.AddColumn(t_("Value"));
	attrs.ColumnWidths("1 1");
	attrs.WhenCursor << THISBACK(DataAttribute);

	colors.AddColumn(t_("Colors"));
	colors.WhenCursor << THISBACK(DataColor);

	actions.AddColumn(t_("Action"));
	actions.AddColumn(t_("Count"));
	actions.ColumnWidths("3 1");
	actions.WhenCursor << THISBACK(DataAction);

	action_args.AddColumn(t_("Action args"));
	action_args.AddColumn(t_("Count"));
	action_args.ColumnWidths("3 1");
	action_args.WhenCursor << THISBACK(DataActionHeader);

	parts.AddColumn(t_("Phrase"));
	parts.AddColumn(t_("Actions"));
	parts.AddColumn(t_("Group"));
	parts.AddColumn(t_("Value"));
	parts.AddColumn(t_("Scores")).SetDisplay(Single<ScoreDisplay>());
	parts.AddColumn(t_("Score-sum"));
	parts.AddIndex("IDX");
	parts.ColumnWidths("8 16 6 6 3 1");
	parts.WhenBar << [this](Bar& bar){
		bar.Add("Copy", [this]() {
			int i = parts.GetCursor();
			AttrText text = parts.Get(i, 3);
			String s = text.text.ToString();
			WriteClipboardText(s);
		});
	};

}

void PhrasePartAnalysis::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		attrs.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	// Set attributes
	attrs.Set(0,0, "All");
	for(int i = 0; i < src.attrs.GetCount(); i++) {
		const AttrHeader& ah = src.attrs.GetKey(i);
		attrs.Set(1+i, 0, ah.group);
		attrs.Set(1+i, 1, ah.value);
	}
	INHIBIT_CURSOR(attrs);
	attrs.SetCount(src.attrs.GetCount());
	if (!attrs.IsCursor() && attrs.GetCount())
		attrs.SetCursor(0);


	DataAttribute();
}

void PhrasePartAnalysis::DataAttribute() {
	colors.SetCount(1+GetColorGroupCount());
	colors.Set(0, 0, t_("All words"));
	for(int i = 0; i < GetColorGroupCount(); i++) {
		colors.Set(1+i, 0,
			AttrText("#" + IntStr(i))
				.NormalPaper(GetGroupColor(i)).NormalInk(Black())
				.Paper(Blend(GrayColor(), GetGroupColor(i))).Ink(White()));
	}
	if (colors.GetCount() && !colors.IsCursor())
		colors.SetCursor(0);


	DataColor();
}

void PhrasePartAnalysis::DataColor() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		actions.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	uniq_acts.Clear();
	for (const ActionHeader& ah : src.actions.GetKeys()) {
		uniq_acts.GetAdd(ah.action).GetAdd(ah.arg, 0)++;
	}
	struct Sorter {
		bool operator()(const VectorMap<String, int>& a, const VectorMap<String, int>& b) const {
			return a.GetCount() > b.GetCount();
		}
	};
	SortByValue(uniq_acts, Sorter());
	for (auto& v : uniq_acts.GetValues())
		SortByValue(v, StdGreater<int>());

	actions.Set(0, 0, "All");
	actions.Set(0, 1, src.actions.GetCount());
	for(int i = 0; i < uniq_acts.GetCount(); i++) {
		actions.Set(1+i, 0, uniq_acts.GetKey(i));
		actions.Set(1+i, 1, uniq_acts[i].GetCount());
	}
	actions.SetCount(1+uniq_acts.GetCount());
	if (!actions.IsCursor() && actions.GetCount())
		actions.SetCursor(0);

	DataAction();
}

void PhrasePartAnalysis::DataAction() {
	if (!actions.IsCursor())
		return;

	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		action_args.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	if (actions.IsCursor()) {
		String action = actions.Get(0);
		int i = uniq_acts.Find(action);
		if (i < 0) {
			action_args.SetCount(1);
			action_args.Set(0, 0, "All");
			action_args.Set(0, 1, src.actions.GetCount());
		}
		else {
			auto& args = uniq_acts[i];
			action_args.Set(0, 0, "All");
			action_args.Set(0, 1, args.GetCount());
			for(int i = 0; i < args.GetCount(); i++) {
				action_args.Set(1+i, 0, args.GetKey(i));
				action_args.Set(1+i, 1, args[i]);
			}
			action_args.SetCount(1+args.GetCount());
		}
		if (!action_args.IsCursor() && action_args.GetCount())
			action_args.SetCursor(0);
	}

	DataActionHeader();
}

void PhrasePartAnalysis::DataActionHeader() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		parts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	int clr_i = colors.IsCursor() ? colors.GetCursor() : -1;
	int act_i = actions.IsCursor() ? actions.GetCursor() : -1;
	int arg_i = action_args.IsCursor() ? action_args.GetCursor() : -1;
	int attr_i = attrs.IsCursor() ? attrs.GetCursor() : -1;

	bool clr_filter = clr_i > 0;
	bool attr_filter = attr_i > 0;
	bool action_filter = act_i > 0;
	bool arg_filter = arg_i > 0;

	int match_attr = -1;
	if (attr_filter)
		match_attr = attr_i - 1;

	clr_i--;

	String action_str, arg_str;
	if (action_filter) {
		action_str = actions.Get(0);
		if (arg_filter)
			arg_str = action_args.Get(0);
	}

	int row = 0, max_rows = 10000;
	for(int i = 0; i < src.phrase_parts.GetCount(); i++) {
		PhrasePart& pp = src.phrase_parts[i];

		parts.Set(row, "IDX", i);

		String phrase = src.GetWordString(pp.words);
		parts.Set(row, 0,
			AttrText(phrase)
				.NormalPaper(Blend(pp.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(pp.clr, GrayColor())).Ink(White())
			);

		parts.Set(row, 1, src.GetActionString(pp.actions));


		// Filter by attribute
		if (attr_filter) {
			if (pp.attr >= 0) {
				if (match_attr != pp.attr)
					continue;
			}
			else continue;
		}
		if (pp.attr >= 0) {
			const AttrHeader& ah = src.attrs.GetKey(pp.attr);
			parts.Set(row, 2, ah.group);
			parts.Set(row, 3, ah.value);
		}
		else {
			parts.Set(row, 2, Value());
			parts.Set(row, 3, Value());
		}
		
		
		// Filter by color group
		if (clr_filter && GetColorGroup(pp.clr) != clr_i)
			continue;

		// Filter by action
		if (action_filter) {
			bool found = false;
			for (int ah_i : pp.actions) {
				const ActionHeader& ah = src.actions.GetKey(ah_i);
				if (ah.action == action_str) {
					if (arg_filter) {
						if (ah.arg == arg_str)
							found = true;
					}
					else found = true;
				}
			}
			if (!found)
				continue;
		}

		ValueArray va;
		int sum = 0;
		for(int i = 0; i < SCORE_COUNT; i++) {
			va.Add(pp.scores[i]);
			sum += pp.scores[i];
		}
		parts.Set(row, 4, va);
		parts.Set(row, 5, sum);

		/*parts.Set(row, 2,
			AttrText(ah.action)
				.NormalPaper(Blend(pp.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(pp.clr, GrayColor())).Ink(White())
			);
		parts.Set(row, 3,
			AttrText(ah.arg)
				.NormalPaper(Blend(pp.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(pp.clr, GrayColor())).Ink(White())
			);*/
		row++;

		if (row >= max_rows)
			break;
	}
	parts.SetCount(row);
	parts.SetSortColumn(5, true);

}










PhrasePartAnalysis2::PhrasePartAnalysis2(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << vsplit << parts;
	hsplit.SetPos(2000);
	
	vsplit.Vert() << elements << typecasts << contrasts << colors;
	
	elements.AddColumn(t_("Element"));
	elements.AddColumn(t_("Count"));
	elements.AddIndex("IDX");
	elements.ColumnWidths("3 1");
	elements.WhenCursor << [this]() {
		DataElement();
	};
	
	typecasts.AddColumn(t_("Typeclass"));
	typecasts.AddColumn(t_("Count"));
	typecasts.ColumnWidths("3 1");
	typecasts.WhenCursor << [this]() {
		//DatabaseBrowser::Single().SetAttr(typecasts.GetCursor());
		DataTypeclass();
	};
	
	contrasts.AddColumn(t_("Profile"));
	contrasts.AddColumn(t_("Count"));
	contrasts.ColumnWidths("3 1");
	contrasts.WhenCursor << [this]() {
		//DatabaseBrowser::Single().SetColor(contrasts.GetCursor());
		DataContrast();
	};
	
	colors.AddColumn(t_("Colors"));
	colors.WhenCursor << THISBACK(DataColor);
	
	parts.AddColumn(t_("Phrase"));
	parts.AddColumn(t_("Typeclass"));
	parts.AddColumn(t_("Contrast"));
	/*parts.AddColumn(t_("Profile"));
	parts.AddColumn(t_("Content"));
	parts.AddColumn(t_("Primary"));
	parts.AddColumn(t_("Secondary"));*/
	parts.AddColumn(t_("Score-sum"));
	parts.AddIndex("IDX");
	parts.ColumnWidths("12 12 12 1");
	parts.WhenBar << [this](Bar& bar){
		bar.Add("Copy", [this]() {
			int i = parts.GetCursor();
			AttrText text = parts.Get(i, 3);
			String s = text.text.ToString();
			WriteClipboardText(s);
		});
	};
	
}

void PhrasePartAnalysis2::ClearAll() {
	if (!PromptYesNo(DeQtf("Do you really want to remove all typecasts and contrasts?")))
		return;
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src)
		return;
	auto& src = p.src->Data();
	
	for(int i = 0; i < src.phrase_parts.GetCount(); i++) {
		PhrasePart& pp = src.phrase_parts[i];
		pp.typecasts.Clear();
		pp.contrasts.Clear();
	}
	
	PostCallback(THISBACK(Data));
}

void PhrasePartAnalysis2::Data() {
	DataMain();
}

void PhrasePartAnalysis2::DataMain() {
	DatabaseBrowser& b = DatabaseBrowser::Single();
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		elements.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	// Set elements
	VectorMap<int,int> el_map = src.GetSortedElementsOfPhraseParts();
	
	elements.Set(0, 0, "All");
	elements.Set(0, "IDX", -1);
	for(int i = 0; i < el_map.GetCount(); i++) {
		int el_i = el_map.GetKey(i);
		elements.Set(1+i, 0, src.element_keys[el_i]);
		elements.Set(1+i, 1, el_map[i]);
		elements.Set(1+i, "IDX", el_map.GetKey(i));
	}
	INHIBIT_CURSOR(elements);
	elements.SetCount(1+el_map.GetCount());
	if (!elements.IsCursor() && elements.GetCount())
		elements.SetCursor(0);
	
	DataElement();
}

void PhrasePartAnalysis2::DataElement() {
	if (!elements.IsCursor())
		return;
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		typecasts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	// Set typeclasses
	const auto& tc = src.typeclasses;
	typecasts.Set(0, 0, "All");
	for(int i = 0; i < tc.GetCount(); i++) {
		typecasts.Set(1+i, 0, tc[i]);
		typecasts.Set(1+i, 1, 0);
	}
	INHIBIT_CURSOR(typecasts);
	typecasts.SetCount(1+tc.GetCount());
	//typecasts.SetSortColumn(2, true);
	if (!typecasts.IsCursor() && typecasts.GetCount())
		typecasts.SetCursor(0);
	
	DataTypeclass();
}

void PhrasePartAnalysis2::DataTypeclass() {
	if (!typecasts.IsCursor())
		return;
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		contrasts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	const auto& vec = src.contents;
	contrasts.Set(0, 0, "All");
	for(int i = 0; i < vec.GetCount(); i++) {
		/*DatabaseBrowser::ActionGroup& a = b.groups[i];
		contrasts.Set(i, 0, a.group);
		contrasts.Set(i, 1, a.count);*/
		contrasts.Set(1+i, 0, vec[i].key);
		contrasts.Set(1+i, 1, 0);
	}
	INHIBIT_CURSOR(contrasts);
	contrasts.SetCount(1+vec.GetCount());
	//contrasts.SetSortColumn(1, true);
	if (!contrasts.IsCursor() && contrasts.GetCount())
		contrasts.SetCursor(0);
	
	DataContrast();
}


void PhrasePartAnalysis2::DataContrast() {
	colors.SetCount(1+GetColorGroupCount());
	colors.Set(0, 0, t_("All words"));
	for(int i = 0; i < GetColorGroupCount(); i++) {
		colors.Set(1+i, 0,
			AttrText("#" + IntStr(i))
				.NormalPaper(GetGroupColor(i)).NormalInk(Black())
				.Paper(Blend(GrayColor(), GetGroupColor(i))).Ink(White()));
	}
	if (colors.GetCount() && !colors.IsCursor())
		colors.SetCursor(0);


	DataColor();
}

void PhrasePartAnalysis2::DataColor() {
	if (!typecasts.IsCursor() || !colors.IsCursor())
		return;
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		parts.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	//DatabaseBrowser& b = DatabaseBrowser::Single();
	int el_i = elements.Get("IDX");
	int tc_i = typecasts.GetCursor() - 1;
	int con_i = contrasts.GetCursor() - 1;
	int clr_i = colors.GetCursor() - 1;
	bool clr_filter = clr_i >= 0;
	
	const auto& tc_v = src.typeclasses;
	const auto& con_v = src.contents;
	
	//int count = min(b.data.GetCount(), 10000);
	int count = src.phrase_parts.GetCount();
	int row = 0;
	for(int i = 0; i < count; i++) {
		/*int pp_i = b.data[i];
		int row = i;
		PhrasePart& pp = src.phrase_parts[pp_i];*/
		int pp_i = i;
		PhrasePart& pp = src.phrase_parts[i];
		
		if (el_i >= 0) {
			bool found = pp.el_i == el_i;
			if (!found) continue;
		}
		if (tc_i >= 0) {
			bool found = false;
			for (int j : pp.typecasts)
				if (j == tc_i)
					{found = true; break;}
			if (!found) continue;
		}
		if (con_i >= 0) {
			bool found = false;
			for (int j : pp.contrasts)
				if (j == con_i)
					{found = true; break;}
			if (!found) continue;
		}
		
		// Filter by color group
		if (clr_filter && GetColorGroup(pp.clr) != clr_i)
			continue;
		
		{
			String s;
			for (int j : pp.typecasts)
				s << tc_v[j] << ", ";
			parts.Set(row, 1, s);
		}
		{
			String s;
			for (int j : pp.contrasts) {
				int con_i = j / PART_COUNT;
				int con_j = j % PART_COUNT;
				if (con_i < con_v.GetCount())
					s << con_v[con_i].key << " #" << (con_j+1) << ", ";
			}
			parts.Set(row, 2, s);
		}
		
		parts.Set(row, "IDX", pp_i);
		
		String phrase = src.GetWordString(pp.words);
		parts.Set(row, 0,
			AttrText(phrase)
				.NormalPaper(Blend(pp.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(pp.clr, GrayColor())).Ink(White())
			);
		
		int sum = 0;
		for(int i = 0; i < SCORE_COUNT; i++) {
			sum += pp.scores[i];
		}
		
		parts.Set(row, 3, sum);
		
		row++;
		
		
		if (row >= 10000)
			break;
	}
	parts.SetCount(row);
	parts.SetSortColumn(3, true);
	
}

void PhrasePartAnalysis2::UpdateCounts() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		return;
	}
	auto& src = p.src->Data();
	
	TODO
	/*int count = src.phrase_parts.GetCount();
	int row = 0;
	for(int i = 0; i < count; i++) {
		PhrasePart& pp = src.phrase_parts[i];
		pp.profiles.Clear();
		pp.primary.Clear();
		pp.secondary.Clear();
	}*/
	/*for (ExportAction& ea : src.primaries.GetValues())
		ea.count = 0;
	for (ExportAttr& ea : src.typecasts.GetValues())
		ea.count = 0;
	
	for(int i = 0; i < src.phrase_parts.GetCount(); i++) {
		PhrasePart& pp = src.phrase_parts[i];
		
		for (int ah_i : pp.primaries) {
			ExportAction& ea = src.primaries[ah_i];
			ea.count++;
		}
		
		if (pp.attr >= 0) {
			ExportAttr& ea = src.typecasts[pp.attr];
			ea.count++;
		}
	}*/
}






ActionAttrsPage::ActionAttrsPage(SourceTextCtrl& o) : o(o) {
	Add(hsplit.VSizePos(0,30).HSizePos());
	
	hsplit.Horz() << vsplit << actions;
	hsplit.SetPos(2000);
	
	vsplit.Vert() << attrs << colors;
	
	attrs.AddColumn(t_("Group"));
	attrs.AddColumn(t_("Value"));
	attrs.AddIndex("GROUP");
	attrs.AddIndex("VALUE");
	attrs.ColumnWidths("1 1");
	attrs.WhenCursor << THISBACK(DataAttribute);
	
	colors.AddColumn(t_("Colors"));
	colors.WhenCursor << THISBACK(DataColor);
	
	actions.AddColumn(t_("Group"));
	actions.AddColumn(t_("Value"));
	actions.AddColumn(t_("Action"));
	actions.AddColumn(t_("Action arg"));
	actions.AddIndex("IDX");
	actions.ColumnWidths("2 2 4 4");
}

void ActionAttrsPage::Data() {
	int gi = 0;
	int i = 0;
	
	attrs.Set(i, 0, "All");
	attrs.Set(i, 1, "All");
	attrs.Set(i, "GROUP", -1);
	attrs.Set(i, "VALUE", -1);
	i++;
	
	#define ATTR_ITEM(e, g, i0, i1) \
		attrs.Set(i, 0, g); \
		attrs.Set(i, 1, i0); \
		attrs.Set(i, "GROUP", gi); \
		attrs.Set(i, "VALUE", 0); \
		i++; \
		attrs.Set(i, 0, g); \
		attrs.Set(i, 1, i1); \
		attrs.Set(i, "GROUP", gi); \
		attrs.Set(i, "VALUE", 1); \
		i++, gi++;
	ATTR_LIST
	#undef ATTR_ITEM

	if (!attrs.IsCursor() && attrs.GetCount())
		attrs.SetCursor(0);
	
	
	DataAttribute();
}

void ActionAttrsPage::DataAttribute() {
	if (!attrs.IsCursor())
		return;
	
	
	
	colors.SetCount(1+GetColorGroupCount());
	colors.Set(0, 0, t_("All words"));
	for(int i = 0; i < GetColorGroupCount(); i++) {
		colors.Set(1+i, 0,
			AttrText("#" + IntStr(i))
				.NormalPaper(GetGroupColor(i)).NormalInk(Black())
				.Paper(Blend(GrayColor(), GetGroupColor(i))).Ink(White()));
	}
	if (colors.GetCount() && !colors.IsCursor())
		colors.SetCursor(0);
	
	
	DataColor();
}

void ActionAttrsPage::DataColor() {
	if (!colors.IsCursor() || !attrs.IsCursor())
		return;
	
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		actions.Clear();
		return;
	}
	auto& src = p.src->Data();
	int clr_i = colors.GetCursor();
	int attr_group_i = attrs.Get("GROUP");
	int attr_value_i = attrs.Get("VALUE");
	String group_str = attr_group_i >= 0 ? ToLower(AttrKeys[attr_group_i][1]) : String();
	String value_str = attr_group_i >= 0 ? ToLower(AttrKeys[attr_group_i][2 + attr_value_i]) : String();
	
	bool clr_filter = clr_i > 0;
	bool attr_filter = attr_group_i >= 0;
	clr_i--;
	
	int row = 0, max_rows = 10000;
	for(int i = 0; i < src.actions.GetCount(); i++) {
		const ActionHeader& ah = src.actions.GetKey(i);
		ExportAction& aa = src.actions[i];
		
		// Filter by color group
		if (clr_filter && GetColorGroup(aa.clr) != clr_i)
			continue;
		
		// Filter by attribute
		String g, v;
		if (attr_filter) {
			if (aa.attr < 0)
				continue;
			const AttrHeader& ath = src.attrs.GetKey(aa.attr);
			if (ath.group != group_str || ath.value != value_str)
				continue;
		}
		
		
		actions.Set(row, "IDX", i);
		if (aa.attr >= 0) {
			const AttrHeader& ath = src.attrs.GetKey(aa.attr);
			actions.Set(row, 0, ath.group);
			actions.Set(row, 1, ath.value);
		}
		else {
			actions.Set(row, 0, Value());
			actions.Set(row, 1, Value());
		}
		actions.Set(row, 2,
			AttrText(ah.action)
				.NormalPaper(Blend(aa.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(aa.clr, GrayColor())).Ink(White())
			);
		actions.Set(row, 3,
			AttrText(ah.arg)
				.NormalPaper(Blend(aa.clr, White(), 128+64)).NormalInk(Black())
				.Paper(Blend(aa.clr, GrayColor())).Ink(White())
			);
		row++;
		if (row >= max_rows)
			break;
	}
	actions.SetCount(row);
	actions.SetSortColumn(2);
	
}







Attributes::Attributes(SourceTextCtrl& o) : o(o) {
	Add(hsplit.SizePos());
	
	hsplit.Horz() << groups << values << vsplit;
	hsplit.SetPos(1000,0);
	
	vsplit.Vert() << pos_values << neg_values;
	
	groups.AddColumn(t_("Group"));
	groups.AddColumn(t_("Value count"));
	groups.AddColumn(t_("Link"));
	groups.AddIndex("IDX");
	groups.ColumnWidths("3 1 1");
	groups.WhenCursor << THISBACK(DataGroup);
	
	values.AddColumn(t_("Value"));
	
	pos_values.AddColumn(t_("Value"));
	pos_values.AddColumn(t_("Most popular"));
	
	neg_values.AddColumn(t_("Value"));
	neg_values.AddColumn(t_("Most popular"));
	
	
}

void Attributes::RealizeTemp() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src)
		return;
	auto& src = p.src->Data();
	
	uniq_attrs.Clear();
	uniq_attrs_i.Clear();
	
	{
		for(int i = 0; i < src.attrs.GetCount(); i++) {
			const AttrHeader& ah = src.attrs.GetKey(i);
			uniq_attrs.GetAdd(ah.group).FindAdd(ah.value);
			uniq_attrs_i.GetAdd(ah.group).FindAdd(i);
		}
		
		SortByKey(uniq_attrs, StdLess<String>());
		for(int i = 0; i < uniq_attrs.GetCount(); i++) {
			SortIndex(uniq_attrs[i], StdLess<String>());
		}
	}
}

void Attributes::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		groups.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	RealizeTemp();
	
	for(int i = 0; i < uniq_attrs.GetCount(); i++) {
		String group = uniq_attrs.GetKey(i);
		const auto& v = uniq_attrs[i];
		groups.Set(i, 0, group);
		groups.Set(i, 1, v.GetCount());
		groups.Set(i, "IDX", i);
		
		if (v.GetCount() == 1) {
			AttrHeader ah;
			ah.group = group;
			ah.value = v[0];
			int j = src.attrs.Find(ah);
			if (j >= 0) {
				const ExportAttr& ea = src.attrs[j];
				if (ea.link >= 0) {
					const AttrHeader& link_ah = src.attrs.GetKey(ea.link);
					groups.Set(i, 2, link_ah.group + ": " + link_ah.value);
				}
			}
			else
				groups.Set(i, 2, Value());
		}
		else {
			groups.Set(i, 2, Value());
		}
	}
	INHIBIT_CURSOR(groups);
	groups.SetCount(uniq_attrs.GetCount());
	groups.SetSortColumn(1, true);
	if (!groups.IsCursor() && groups.GetCount())
		groups.SetCursor(0);
	
	DataGroup();
}

void Attributes::DataGroup() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		values.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	RealizeTemp();
	
	if (!groups.IsCursor()) {
		pos_values.Clear();
		neg_values.Clear();
		return;
	}
	
	int g_i = groups.Get("IDX");
	const String& group = uniq_attrs.GetKey(g_i);
	const Index<String>& gvalues = uniq_attrs[g_i];
	
	int attr_i0 = -1;
	int attr_i1 = -1;
	String attr_s0, attr_s1;
	int i = src.simple_attrs.Find(group);
	if (i >= 0) {
		const ExportSimpleAttr& t = src.simple_attrs[i];
		attr_i0 = t.attr_i0;
		attr_i1 = t.attr_i1;
		attr_s0 = src.attrs.GetKey(attr_i0).value;
		attr_s1 = src.attrs.GetKey(attr_i1).value;
	}
	for(int i = 0; i < gvalues.GetCount(); i++) {
		String attr_s = gvalues[i];
		if (attr_s == attr_s0)
			SetColoredListValue(values, i, 0, attr_s, Green());
		else if (attr_s == attr_s1)
			SetColoredListValue(values, i, 0, attr_s, Red());
		else
			values.Set(i, 0, attr_s);
	}
	INHIBIT_CURSOR(values);
	values.SetCount(gvalues.GetCount());
	if (!values.IsCursor() && values.GetCount())
		values.SetCursor(0);
	
	i = uniq_attrs_i.Find(group);
	if (i < 0) {
		pos_values.SetCount(0);
		neg_values.SetCount(0);
	}
	else {
		const auto& indices = uniq_attrs_i[i];
		int prow = 0, nrow = 0;
		for(int i = 0; i < indices.GetCount(); i++) {
			const AttrHeader& ah = src.attrs.GetKey(indices[i]);
			const ExportAttr& ea = src.attrs[indices[i]];
			if (ea.positive) {
				pos_values.Set(prow, 0, ah.value);
				prow++;
			}
			else {
				neg_values.Set(nrow, 0, ah.value);
				nrow++;
			}
			pos_values.SetCount(prow);
			neg_values.SetCount(nrow);
		}
	}
	
}







TextDataDiagnostics::TextDataDiagnostics(SourceTextCtrl& o) : o(o) {
	Add(hsplit.HSizePos().VSizePos(0,30));
	
	hsplit.Horz() << values;
	
	values.AddColumn(t_("Key"));
	values.AddColumn(t_("Value"));
}

void TextDataDiagnostics::Data() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		values.Clear();
		return;
	}
	auto& src = p.src->Data();
	
	for(int i = 0; i < src.diagnostics.GetCount(); i++) {
		const String& key = src.diagnostics.GetKey(i);
		const String& value = src.diagnostics[i];
		
		values.Set(i, 0, key);
		
		if (key.Find("percentage") >= 0 && key.Find("fail percentage") < 0) {
			double perc = StrDbl(value);
			if (perc < 50)
				SetColoredListValue(values, i, 1, Format("%.3m", perc), Red());
			else
				values.Set(i, 1, value);
		}
		else values.Set(i, 1, value);
	}
	values.SetCount(src.diagnostics.GetCount());
	
}







SourceTextCtrl::SourceTextCtrl() :
	tk(*this),
	src(*this),
	awp(*this),
	vp(*this),
	vpp(*this),
	vps(*this),
	vpa(*this),
	vpa2(*this),
	aap(*this),
	att(*this),
	diag(*this)
{
	data_type.Add("Source");
	data_type.Add("Analyzed");
	data_type.Add("Tokens");
	data_type.Add("Ambiguous Word Pairs");
	data_type.Add("Virtual Phrases");
	data_type.Add("Virtual Phrase Parts");
	data_type.Add("Virtual Phrase Structs");
	data_type.Add("Phrase Part Analysis 1");
	data_type.Add("Phrase Part Analysis 2");
	data_type.Add("Action Attrs Page");
	data_type.Add("Attributes");
	data_type.Add("Text Data Diagnostics");
	data_type.SetIndex(0);
	data_type.WhenAction << THISBACK(SetDataCtrl);
	PostCallback(THISBACK(SetDataCtrl));
	
	Add(prog.BottomPos(0,30).HSizePos(300));
	Add(remaining.BottomPos(0,30).LeftPos(0,300));
	AddMenu();
	
}

void SourceTextCtrl::SetFont(Font fnt) {
	src.SetFont(fnt);
	//tk.SetFont(fnt);
}

void SourceTextCtrl::SetDataCtrl() {
	RemoveChild(&src);
	RemoveChild(&tk);
	RemoveChild(&awp);
	RemoveChild(&vp);
	RemoveChild(&vpp);
	RemoveChild(&vps);
	RemoveChild(&vpa);
	RemoveChild(&vpa2);
	RemoveChild(&aap);
	RemoveChild(&att);
	RemoveChild(&diag);
	
	int idx = data_type.GetIndex();
	switch (idx) {
		case 0:
		case 1: Add(src.SizePos()); break;
		case 2: Add(tk.SizePos()); break;
		case 3: Add(awp.SizePos()); break;
		case 4: Add(vp.SizePos()); break;
		case 5: Add(vpp.SizePos()); break;
		case 6: Add(vps.SizePos()); break;
		case 7: Add(vpa.SizePos()); break;
		case 8: Add(vpa2.SizePos()); break;
		case 9: Add(aap.SizePos()); break;
		case 10: Add(att.SizePos()); break;
		case 11: Add(diag.SizePos()); break;
	}
	Data();
}

void SourceTextCtrl::Data() {
	int idx = data_type.GetIndex();
	switch (idx) {
		case 0:
		case 1: src.SetDataType(idx); src.Data(); break;
		case 2: tk.Data(); break;
		case 3: awp.Data(); break;
		case 4: vp.Data(); break;
		case 5: vpp.Data(); break;
		case 6: vps.Data(); break;
		case 7: vpa.Data(); break;
		case 8: vpa2.Data(); break;
		case 9: aap.Data(); break;
		case 10: att.Data(); break;
		case 11: diag.Data(); break;
	}
}

void SourceTextCtrl::OnLoad(const String& data, const String& filepath) {
	MetaEnv().LoadFileRootJson("", filepath, data, true);
}

void SourceTextCtrl::OnSave(String& data, const String& filepath) {
	
	LOG("warning: skipping saving");
	
}

void SourceTextCtrl::ToolMenu(Bar& bar) {
	// TODO improve gui look and user experience
	bar.Add(data_type, Size(200,24));
	bar.Separator();
	bar.Add(t_("Refresh"), TextImgs::BlueRing(), THISBACK(Data)).Key(K_CTRL_Q);
	bar.Separator();
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
}

void SourceTextCtrl::Do(int fn) {
	int page = data_type.GetIndex();
	switch (page) {
		case 0: DoT<SourceDataImporter>(fn - 0); break;
		case 1: DoT<SourceAnalysisProcess>(fn - 2); break;
		case 2: DoT<TokenDataProcess>(fn - 4); break;
		case 3: DoT<AmbiguousWordPairsProcess>(fn); break; // awp
		case 4: DoT<VirtualPhrasesProcess>(fn); break; // vp
		case 5: DoT<VirtualPhrasePartsProcess>(fn); break; // vpp
		case 6: DoT<VirtualPhraseStructsProcess>(fn); break; // vps
		case 7: DoT<PhrasePartAnalysisProcess>(fn); break; // vpa
		case 8: DoT<PhrasePartAnalysisProcess>(fn); break; // vpa2
		case 9: DoT<ActionAttrsProcess>(fn); break; // aap
		case 10: DoT<AttributesProcess>(fn); break; // att
		case 11: break; // diag
		default: break;
	}
}


END_UPP_NAMESPACE
