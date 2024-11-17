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
	components.WhenCursor << THISBACK(DataComponent);
	
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
	auto& src = *p.src;
	const auto& data = src.src_entities;
	
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
	auto& src = *p.src;
	
	if (!entities.IsCursor()) return;
	int acur = entities.GetCursor();
	const auto& data = src.src_entities;
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
	
	DataComponent();
}

void SourceDataCtrl::DataComponent() {
	DatasetPtrs& p = o.GetDataset();
	if (!p.src) {
		analysis.Clear();
		return;
	}
	auto& src = *p.src;
	
	if (!entities.IsCursor() || !components.IsCursor()) return;
	int acur = entities.GetCursor();
	int scur = components.GetCursor();
	const auto& data = src.src_entities;
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
		String txt = p.GetScriptDump(ss_i);
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
	auto& src = *p.src;
	
	for(int j = 0; j < src.tokens.GetCount(); j++) {
		const String& txt = src.tokens.GetKey(j);
		const Token& tk = src.tokens[j];
		
		tokens.Set(j, 0, txt);
	}
	tokens.SetCount(src.tokens.GetCount());
	
}
















SourceTextCtrl::SourceTextCtrl() : src(*this), tk(*this) {
	data_type.Add("Source");
	data_type.Add("Analyzed");
	data_type.Add("Tokens");
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
	int idx = data_type.GetIndex();
	switch (idx) {
		case 0:
		case 1: Add(src.SizePos()); break;
		case 2: Add(tk.SizePos()); break;
	}
	Data();
}

void SourceTextCtrl::Data() {
	int idx = data_type.GetIndex();
	switch (idx) {
		case 0:
		case 1: src.SetDataType(idx); src.Data(); break;
		case 2: tk.Data(); break;
	}
}

void SourceTextCtrl::OnLoad(const String& data, const String& filepath) {
	String compressed;
	StringStream comp_stream(data);
	comp_stream % this->data_sha1 % compressed;
	String decompressed = BZ2Decompress(compressed);
	StringStream decomp_stream(decompressed);
	SrcTextData* src;
	int i = DatasetIndex().Find(filepath);
	if (i < 0) {
		src = new SrcTextData;
		DatasetIndex().Add(filepath, src);
	}
	else src = dynamic_cast<SrcTextData*>(&DatasetIndex()[i]);
	src->Serialize(decomp_stream);
	DatasetPtrs& p = GetDataset();
	p.src = src;
}

void SourceTextCtrl::OnSave(String& data, const String& filepath) {
	int i = DatasetIndex().Find(filepath);
	ASSERT(i >= 0);
	SrcTextData* src = dynamic_cast<SrcTextData*>(&DatasetIndex()[i]);
	StringStream decomp_stream;
	src->Serialize(decomp_stream);
	String decompressed = decomp_stream.GetResult();
	this->data_sha1 = SHA1String(decompressed);
	String compressed = BZ2Compress(decompressed);
	StringStream comp_stream;
	comp_stream % this->data_sha1 % compressed;
	data = comp_stream.GetResult();
}

void SourceTextCtrl::ToolMenu(Bar& bar) {
	// TODO improve gui look and user experience
	bar.Add(data_type, Size(200,24));
	bar.Separator();
	bar.Add(t_("Update Data"), TextImgs::BlueRing(), THISBACK(Data)).Key(K_CTRL_Q);
	bar.Separator();
	bar.Add(t_("Start import"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop import"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	bar.Separator();
	bar.Add(t_("Start source analysis"), TextImgs::RedRing(), THISBACK1(Do, 2)).Key(K_F7);
	bar.Add(t_("Stop source analysis"), TextImgs::RedRing(), THISBACK1(Do, 3)).Key(K_F8);
	bar.Separator();
	bar.Add(t_("Start token data processing"), TextImgs::RedRing(), THISBACK1(Do, 4)).Key(K_F9);
	bar.Add(t_("Stop token data processing"), TextImgs::RedRing(), THISBACK1(Do, 5)).Key(K_F10);
}

void SourceTextCtrl::Do(int fn) {
	if (fn < 2) {
		DoT<SourceDataImporter>(fn - 0);
	}
	else if (fn < 4) {
		DoT<SourceAnalysisProcess>(fn - 2);
	}
	else if (fn < 6) {
		DoT<TokenDataProcess>(fn - 4);
	}
}


END_UPP_NAMESPACE
