#include "TextCtrl.h"

NAMESPACE_UPP


SourceDataCtrl::SourceDataCtrl() {
	Add(hsplit.VSizePos(0,30).HSizePos());
	Add(prog.BottomPos(0,30).HSizePos(300));
	Add(remaining.BottomPos(0,30).LeftPos(0,300));
	
	hsplit.Horz() << vsplit << scripts << analysis;
	hsplit.SetPos(2500);
	
	vsplit.Vert() << entities << components;
	
	entities.AddColumn(t_("File"));
	entities.WhenCursor << THISBACK(DataEntity);
	
	components.AddColumn(t_("Entry"));
	components.WhenCursor << THISBACK(DataComponent);
	
}

void SourceDataCtrl::SetFont(Font fnt) {
	scripts.SetFont(fnt);
	analysis.SetFont(fnt);
	remaining.SetFont(fnt);
}

void SourceDataCtrl::OnLoad(const String& data, const String& filepath) {
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

void SourceDataCtrl::OnSave(String& data, const String& filepath) {
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

void SourceDataCtrl::Data() {
	DatasetPtrs& p = GetDataset();
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
	DatasetPtrs& p = GetDataset();
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
	DatasetPtrs& p = GetDataset();
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
	
	String s = song.text;
	if (GetDefaultCharset() != CHARSET_UTF8)
		s = ToCharset(CHARSET_DEFAULT, s, CHARSET_UTF8);
	this->scripts.SetData(s);
	analysis.Clear();
	
	
	TryNo5tStructureSolver solver;
	solver.Process(s);
	analysis.SetData(solver.GetResult());
	//analysis.SetData(solver.GetDebugLines());
	
	TODO
	// get data from SourceAnalysisCtrl
}

void SourceDataCtrl::ToolMenu(Bar& bar) {
	bar.Add(t_("Start"), TextImgs::RedRing(), THISBACK1(Do, 0)).Key(K_F5);
	bar.Add(t_("Stop"), TextImgs::RedRing(), THISBACK1(Do, 1)).Key(K_F6);
	#error join 3 pages to 1
}

void SourceDataCtrl::Do(int fn) {
	DatasetPtrs p;
	TODO
	
	SourceDataImporter& sdi = SourceDataImporter::Get(p);
	prog.Attach(sdi);
	sdi.WhenRemaining << [this](String s) {PostCallback([this,s](){remaining.SetLabel(s);});};
	
	if (fn == 0)
		sdi.Start();
	else
		sdi.Stop();
}


END_UPP_NAMESPACE
