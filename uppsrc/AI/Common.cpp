#include "AI.h"

NAMESPACE_UPP


AiAnnotationItem::SourceFile& AiAnnotationItem::RealizeFileByHashSha1(const String& sha1)
{
	Mutex::Lock ml(lock);
	for (SourceFile& sf : source_files) {
		if (sf.file_hash_sha1 == sha1) {
			return sf;
		}
	}
	SourceFile& sf = source_files.Add();
	sf.file_hash_sha1 = sha1;
	return sf;
}

int AiAnnotationItem::FindAddData(const String& txt)
{
	Mutex::Lock ml(lock);
	hash_t h = txt.GetHashValue();
	int i = 0;
	for (Data& d : data) {
		if (d.tmp_hash == 0) d.tmp_hash = d.txt.GetHashValue();
		if (d.tmp_hash == h) {
			return i;
		}
		i++;
	}
	i = data.GetCount();
	Data& d = data.Add();
	d.txt = txt;
	d.tmp_hash = d.txt.GetHashValue();
	return i;
}

int AiAnnotationItem::GetDataCount() const
{
	Mutex::Lock ml(lock);
	int i = data.GetCount();
	return i;
}

String AiAnnotationItem::GetDataString(int data_i) const
{
	Mutex::Lock ml(lock);
	String s = data[data_i].txt;
	return s;
}

void AiAnnotationItem::operator=(const AiAnnotationItem& s)
{
	Mutex::Lock ml(lock);
	
	kind = s.kind;
	definition = s.definition;
	isvirtual = s.isvirtual;
	isstatic = s.isstatic;
	name = s.name;
	type = s.type;
	id = s.id;
	pretty = s.pretty;
	nspace = s.nspace;
	uname = s.uname;
	nest = s.nest;
	unest = s.unest;
	bases = s.bases;
	data <<= s.data;
	source_files <<= source_files;
	
}

void AiAnnotationItem::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	
	json	("k", kind)
			("d", definition)
			("iv", isvirtual)
			("is", isstatic)
			("n", name)
			("t", type)
			("i", id)
			("r", pretty)
			("s", nspace)
			("u", uname)
			("q", nest)
			("w", unest)
			("y", bases)
			("da", data)
			("sf", source_files)
			;
	//if (json.IsLoading()) Sort();
}

void AiAnnotationItem::Serialize(Stream& s)
{
	//if (s.IsStoring()) Sort();
	Mutex::Lock ml(lock);
	
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % kind
		  % definition
		  % isvirtual
		  % isstatic
		  % name
		  % type
		  % id
		  % pretty
		  % nspace
		  % uname
		  % nest
		  % unest
		  % bases
		  % data
		  % source_files
		  ;
	}
	
}

void AiAnnotationItem::SourceFile::Sort() {
	Mutex::Lock ml(lock);
	UPP::Sort(items, Item());
}

void AiAnnotationItem::Sort() {
	for (SourceFile& f : source_files)
		f.Sort();
}

void AiAnnotationItem::Data::operator=(const Data& f)
{
	txt = f.txt;
	tmp_hash = f.tmp_hash;
}

void AiAnnotationItem::Data::Jsonize(JsonIO& json)
{
	json
		("s", txt)
		;
}

void AiAnnotationItem::Data::Serialize(Stream& s)
{
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % txt;
	}
}

void AiAnnotationItem::SourceFile::Item::Jsonize(JsonIO& json)
{
	json
		("k", (int&)kind)
		("l", rel_line)
		("d", data_i)
		;
}

void AiAnnotationItem::SourceFile::Item::Serialize(Stream& s)
{
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % (int&)kind
		  % rel_line
		  % data_i
		  ;
	}
}

void AiAnnotationItem::SourceFile::operator=(const SourceFile& f)
{
	Mutex::Lock ml(lock);
	items <<= f.items;
	file_hash_sha1 = f.file_hash_sha1;
	pos = f.pos;
	begin = f.begin;
	end = f.end;
}

void AiAnnotationItem::SourceFile::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	json
		("items", items)
		("h", file_hash_sha1)
		("p", pos)
		("b", begin)
		("e", end)
		;
}

void AiAnnotationItem::SourceFile::Serialize(Stream& s)
{
	Mutex::Lock ml(lock);
	
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % items
		  % file_hash_sha1
		  % pos
		  % begin
		  % end;
	}
}

void AiFileInfo::operator=(const AiFileInfo& s)
{
	Mutex::Lock ml(lock);
	ai_items <<= s.ai_items;
}

void AiFileInfo::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	json("items", ai_items);
	//if (json.IsLoading()) Sort();
}

void AiFileInfo::Serialize(Stream& s)
{
	Mutex::Lock ml(lock);
	
	byte version = 1;
	s % version;
	
	if (version >= 1)
		s % ai_items;
	
}

void AiFileInfo::UpdateLinks(FileAnnotation& ann)
{
	Mutex::Lock ml(lock);
	
	int c0 = ann.items.GetCount();
	int c1 = ai_items.GetCount();
	if(!c1) {
		ai_items.SetCount(c0);
		for(int i = 0; i < c0; i++) {
			AnnotationItem& it0 = ann.items[i];
			AiAnnotationItem& it1 = ai_items[i];
			(AnnotationItem&)it1 = ann.items[i];
			// it1.linked = &it0;
		}
	}
	else {
		Vector<bool> linked0, linked1;
		linked0.SetCount(c0, false);
		linked1.SetCount(c1, false);
		int nonlinked0 = c0;
		int nonlinked1 = c1;

		// for (auto& it : ai_items)
		//	it.linked = 0;

		for(int tries = 0; tries < 2; tries++) {
			for(int i = 0; i < ann.items.GetCount(); i++) {
				if(linked0[i])
					continue;
				AnnotationItem& it0 = ann.items[i];
				for(int j = 0; j < ai_items.GetCount(); j++) {
					if(linked1[j])
						continue;
					AiAnnotationItem& it1 = ai_items[j];
					if((tries == 0 && it0.IsSameContent((AnnotationItem&)it1)) ||
					   (tries == 1 && it0.IsLineAreaPartialMatch((AnnotationItem&)it1))) {
						// it1.linked = &it0;
						linked0[i] = true;
						linked1[j] = true;
						nonlinked0--;
						nonlinked1--;
						break;
					}
				}
			}
		}

		for(int i = 0; i < ann.items.GetCount(); i++) {
			if(linked0[i])
				continue;
			AnnotationItem& it0 = ann.items[i];
			AiAnnotationItem& it1 = ai_items.Add();
			// it1.linked = &it0;
			(AnnotationItem&)it1 = it0;
			nonlinked0--;
		}
	}
}

void AiAnnotationItem::SourceFile::RemoveAll(Item::Kind kind) {
	Vector<int> rmlist;
	Mutex::Lock ml(lock);
	for(int i = 0; i < items.GetCount(); i++) {
		if (items[i].kind == kind)
			rmlist << i;
	}
	if (!rmlist.IsEmpty()) items.Remove(rmlist);
}

void AiAnnotationItem::SourceFile::RemoveLineItem(int rel_line)
{
	Vector<int> rm_list;
	Mutex::Lock ml(lock);
	for(int i = 0; i < items.GetCount(); i++) {
		if(items[i].rel_line == rel_line)
			rm_list << i;
	}
	items.Remove(rm_list);
}

AiAnnotationItem::SourceFile::Item* AiAnnotationItem::SourceFile::FindItem(int rel_line)
{
	Mutex::Lock ml(lock);
	for(Item& c : items) {
		if(c.rel_line == rel_line) {
			return &c;
		}
	}
	return 0;
}


END_UPP_NAMESPACE
