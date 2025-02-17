#include "Core.h"

NAMESPACE_UPP

// see SRC_TXT_HEADER_ENABLE

//int EditorPtrs::GetActiveEntityIndex() const {return VectorFindPtr(entity, TextDatabase::Single().entities);}
//int EditorPtrs::GetActiveComponentIndex() const {if (!entity || !component) return -1; return VectorFindPtr(component, entity->comps);}
//int EditorPtrs::GetActiveScriptIndex() const {if (!entity || !script) return -1; return VectorFindPtr(static_cast<Component*>(script), entity->comps);}


void SrcTextData::Serialize(Stream& s) {
	int v = 1; s % v;
	if (v >= 1) {
		s % scripts;					ASSERT(!s.IsError());
		s % tokens;						ASSERT(!s.IsError());
		s % token_texts;				ASSERT(!s.IsError());
		s % element_keys;				ASSERT(!s.IsError());
		s % word_classes;				ASSERT(!s.IsError());
		s % words;						ASSERT(!s.IsError());
		s % ambiguous_word_pairs;		ASSERT(!s.IsError());
		s % virtual_phrases;			ASSERT(!s.IsError());
		s % virtual_phrase_parts;		ASSERT(!s.IsError());
		s % virtual_phrase_structs;		ASSERT(!s.IsError());
		s % phrase_parts;				ASSERT(!s.IsError());
		s % struct_part_types;			ASSERT(!s.IsError());
		s % struct_types;				ASSERT(!s.IsError());
		s % attrs;						ASSERT(!s.IsError());
		s % actions;					ASSERT(!s.IsError());
		s % parallel;					ASSERT(!s.IsError());
		s % trans;						ASSERT(!s.IsError());
		s % action_phrases;				ASSERT(!s.IsError());
		s % wordnets;					ASSERT(!s.IsError());
		s % diagnostics;				ASSERT(!s.IsError());
		s % simple_attrs;				ASSERT(!s.IsError());
		s % entities;					ASSERT(!s.IsError());
		s % typeclasses;				ASSERT(!s.IsError());
		s % contents;					ASSERT(!s.IsError());
		s % content_parts;				ASSERT(!s.IsError());
		s % lang;						ASSERT(!s.IsError());
		for(int i = 0; i < TCENT_COUNT; i++)
			s % typeclass_entities[i];
	}
}

String SrcTextData::GetTokenTextString(const TokenText& txt) const {
	String o;
	for(int tk_i : txt.tokens) {
		//const Token& tk = tokens[tk_i];
		const String& key = tokens.GetKey(tk_i);
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

String SrcTextData::GetTokenTypeString(const TokenText& txt) const {
	String o;
	for(int tk_i : txt.tokens) {
		const Token& tk = this->tokens[tk_i];
		int w_i = tk.word_;
		if (w_i < 0) {
			String key = ToLower(this->tokens.GetKey(tk_i));
			w_i = this->words.Find(key);
			tk.word_ = w_i;
		}
		if (w_i < 0) {
			o << "{error}";
		}
		else {
			const ExportWord& ew = this->words[w_i];
			o << "{";
			for(int i = 0; i < ew.class_count; i++) {
				if (i) o << "|";
				int class_i = ew.classes[i];
				const String& wc = this->word_classes[class_i];
				o << wc;
			}
			o << "}";
			/*if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
				o << key;
			}
			else {
				if (!o.IsEmpty())
					o << " ";
				o << key;
			}*/
		}
	}
	return o;
}

String SrcTextData::GetWordString(const Vector<int>& words) const {
	String o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const String& key = this->words.GetKey(w_i);
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

WString SrcTextData::GetWordPronounciation(const Vector<int>& words) const {
	WString o;
	for(int w_i : words) {
		if (w_i < 0) continue;
		const ExportWord& ew = this->words[w_i];
		const WString& key = ew.phonetic;
		
		if (key.GetCount() == 1 && NaturalTokenizer::IsToken(key[0])) {
			o << key;
		}
		else {
			if (!o.IsEmpty())
				o << " ";
			o << key;
		}
	}
	return o;
}

String SrcTextData::GetTypeString(const Vector<int>& word_classes) const {
	String o;
	for(int wc_i : word_classes) {
		if (wc_i < 0)
			o << "{error}";
		else {
			const String& wc = this->word_classes[wc_i];
			o << "{" << wc << "}";
		}
	}
	return o;
}

String SrcTextData::GetActionString(const Vector<int>& actions) const {
	String o;
	for(int act_i : actions) {
		if (!o.IsEmpty()) o << ", ";
		if (act_i < 0)
			o << "error";
		else {
			const ActionHeader& ah = this->actions.GetKey(act_i);
			o << ah.action;
			if (!ah.arg.IsEmpty())
				o << "(" << ah.arg << ")";
		}
	}
	return o;
}

String SrcTextData::GetScriptDump(int i) const {
	String s;
	const ScriptStruct& ss = this->scripts[i];
	for(int i = 0; i < ss.parts.GetCount(); i++) {
		const auto& part = ss.parts[i];
		//if (s.GetCount()) s << "\n";
		s << Format("[%d: %s]\n", i, GetTextTypeString(part.type) + " " + IntStr(part.num+1));
		
		for(int j = 0; j < part.sub.GetCount(); j++) {
			const auto& sub = part.sub[j];
			//if (s.GetCount()) s << "\n";
			s << Format("\t[%d.%d: repeat %.2!m]\n", i,j, sub.repeat);
			
			bool show_subsub = sub.sub.GetCount() > 1;
			for(int k = 0; k < sub.sub.GetCount(); k++) {
				const auto& ssub = sub.sub[k];
				if (show_subsub)
					s << Format("\t\t[%d.%d.%d]\n", i,j,k);
				for(int l = 0; l < ssub.token_texts.GetCount(); l++) {
					int tt_i = ssub.token_texts[l];
					if (tt_i < 0) continue;
					const TokenText& tt = this->token_texts[tt_i];
					if (show_subsub) s.Cat('\t');
					s << "\t\t" << this->GetTokenTextString(tt) << "\n";
				}
			}
		}
	}
	return s;
}

VectorMap<int,int> SrcTextData::GetSortedElements() {
	VectorMap<int,int> vmap;
	for(const ScriptStruct& ss : this->scripts.GetValues()) {
		for(const auto& part : ss.parts) {
			if (part.cls >= 0)
				vmap.GetAdd(part.cls,0)++;
			for(const auto& sub : part.sub) {
				if (sub.cls >= 0)
					vmap.GetAdd(sub.cls,0)++;
				for(const auto& ssub : sub.sub) {
					if (sub.cls >= 0)
						vmap.GetAdd(sub.cls,0)++;
				}
			}
		}
	}
	SortByValue(vmap, StdGreater<int>());
	return vmap;
}

VectorMap<int,int> SrcTextData::GetSortedElementsOfPhraseParts() {
	VectorMap<int,int> vmap;
	for (const auto& pp : this->phrase_parts.GetValues()) {
		if (pp.el_i >= 0)
			vmap.GetAdd(pp.el_i,0)++;
	}
	SortByValue(vmap, StdGreater<int>());
	return vmap;
}

ArrayMap<String, Ptr<MetaNodeExt>>& DatasetIndex() {
	static ArrayMap<String, Ptr<MetaNodeExt>> map;
	return map;
}



SrcTxtHeader::~SrcTxtHeader() {
	
}

void SrcTxtHeader::Visit(NodeVisitor& v) {
	v.Ver(1)
	(1)	("written",written)
		("size",size)
		("sha1",sha1)
		("files",files);
	
	if (v.IsStoring()) {
		LOG("SrcTxtHeader::Visit: error: storing data not implemented yet");
		v.SetError("SrcTxtHeader::Visit: error: storing data not implemented yet");
	}
}

void SrcTxtHeader::RealizeData() {
	if (!data)
		LoadData();
}

bool SrcTxtHeader::LoadData() {
	ASSERT(!filepath.IsEmpty());
	// if (files.IsEmpty()) {
	//	if (!LoadFromJsonFile(*this, filepath))
	//		return false;
	//}
	ASSERT(files.GetCount());
	
	String compressed;
	String dir = GetFileDirectory(filepath);
	for(int i = 0; i < this->files.GetCount(); i++) {
		String path = AppendFileName(dir, this->files[i]);
		String data = LoadFile(path);
		
		compressed.Cat(data);
		
		int per_file = 1024 * 1024 * 25;
		Logi() << "SrcTxtHeader::LoadData" << data.GetCount() << " vs expected " << per_file << ": " << (data.GetCount() == per_file ? "True" : "False");
	}
	String decompressed = BZ2Decompress(compressed);
	if (decompressed.GetCount() != this->size) {
		Loge() << "SrcTxtHeader::LoadData: error: size mismatch when loading: " << filepath;
		return false;
	}
	String sha1 = SHA1String(decompressed);
	if (sha1 != this->sha1) {
		Loge() << "SrcTxtHeader::LoadData: error: sha1 mismatch when loading: " << filepath;
		return false;
	}
	StringStream decomp_stream(decompressed);
	
	this->data.Create();
	this->data->Serialize(decomp_stream);
	
	return true;
}

String SrcTxtHeader::SaveData() {
	ASSERT(!filepath.IsEmpty());
	
	String dir = GetFileDirectory(filepath);
	String filename = GetFileName(filepath);
	int i = DatasetIndex().Find(filepath);
	ASSERT(i >= 0);
	SrcTextData* src = dynamic_cast<SrcTextData*>(&*DatasetIndex()[i]);
	StringStream decomp_stream;
	src->Serialize(decomp_stream);
	String decompressed = decomp_stream.GetResult();
	
	this->written = GetUtcTime();
	this->sha1 = SHA1String(decompressed);
	this->size = decompressed.GetCount();
	
	String compressed = BZ2Compress(decompressed);
	StringStream comp_stream;
	comp_stream % compressed;
	
	this->files.Clear();
	int per_file = 1024 * 1024 * 25;
	int parts = 1 + (compressed.GetCount() + 1) / per_file;
	for(int i = 0; i < parts; i++) {
		int begin = i * per_file;
		int end = min(begin+per_file, compressed.GetCount());
		String part = compressed.Mid(begin,end-begin);
		String part_path = filepath + "." + IntStr(i);
		FileOut fout(part_path);
		fout.Put(part);
		this->files.Add(filename + "." + IntStr(i));
	}
	for (int i = parts;;) {
		String part_path = filepath + "." + IntStr(i);
		if (FileExists(part_path))
			DeleteFile(part_path);
		else
			break;
	}
	
	return StoreAsJson(*this, true);
}

INITIALIZER_COMPONENT(SrcTxtHeader)

END_UPP_NAMESPACE
