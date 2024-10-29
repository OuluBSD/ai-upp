#include "AI.h"

NAMESPACE_UPP


AiAnnotationItem::SourceRange* AiAnnotationItem::FindRangeByHashSha1(const String& sha1)
{
	Mutex::Lock ml(lock);
	for (SourceRange& sf : source_files) {
		if (sf.range_hash_sha1 == sha1) {
			return &sf;
		}
	}
	return 0;
}

AiAnnotationItem::SourceRange& AiAnnotationItem::RealizeRangeByHashSha1(const String& sha1)
{
	Mutex::Lock ml(lock);
	for (SourceRange& sf : source_files) {
		if (sf.range_hash_sha1 == sha1) {
			return sf;
		}
	}
	SourceRange& sf = source_files.Add();
	sf.range_hash_sha1 = sha1;
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

bool AiAnnotationItem::IsSameContent(const AnnotationItem& b) const
{
	return	id == b.id &&
			name == b.name &&
			type == b.type &&
			pretty == b.pretty &&
			nspace == b.nspace &&
			uname == b.uname &&
			nest == b.nest &&
			unest == b.unest &&
			bases == b.bases &&
			kind == b.kind &&
			definition == b.definition &&
			isvirtual == b.isvirtual &&
			isstatic == b.isstatic;
}

bool AiAnnotationItem::SourceRange::IsLineAreaPartialMatch(const AnnotationItem& b) const {
	return begin.y == b.begin.y && end.y == b.end.y;
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
	source_files <<= s.source_files;
	
}

void AiAnnotationItem::Set(const AnnotationItem& s, const String& range_hash_sha1) {
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
	
	source_files.Clear();
	SourceRange& sf = source_files.Add();
	sf.pos = s.pos;
	sf.begin = s.begin;
	sf.end = s.end;
	sf.range_hash_sha1 = range_hash_sha1;
	
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

void AiAnnotationItem::SourceRange::Sort() {
	Mutex::Lock ml(lock);
	UPP::Sort(items, Item());
}

void AiAnnotationItem::Sort() {
	Mutex::Lock ml(lock);
	for (SourceRange& f : source_files)
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

void AiAnnotationItem::SourceRange::Item::Jsonize(JsonIO& json)
{
	json
		("k", reinterpret_cast<KindType&>(kind))
		("l", rel_line)
		("d", data_i)
		;
}

void AiAnnotationItem::SourceRange::Item::Serialize(Stream& s)
{
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % reinterpret_cast<KindType&>(kind)
		  % rel_line
		  % data_i
		  ;
	}
}

AiAnnotationItem::SourceRange::SourceRange() {
	
}

AiAnnotationItem::SourceRange::SourceRange(const SourceRange& f) {
	*this = f;
}

void AiAnnotationItem::SourceRange::operator=(const SourceRange& f)
{
	Mutex::Lock ml(lock);
	items <<= f.items;
	range_hash_sha1 = f.range_hash_sha1;
	pos = f.pos;
	begin = f.begin;
	end = f.end;
}

void AiAnnotationItem::SourceRange::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	json
		("items", items)
		("h", range_hash_sha1)
		/*("p", pos) // keep this data temporary
		("b", begin)
		("e", end)*/
		;
}

void AiAnnotationItem::SourceRange::Serialize(Stream& s)
{
	Mutex::Lock ml(lock);
	
	byte version = 1;
	s % version;
	
	if (version >= 1) {
		s % items
		  % range_hash_sha1
		  /*% pos  // keep this data temporary
		  % begin
		  % end*/;
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
	
	ASSERT(!ann.path.IsEmpty());
	String content = LoadFile(ann.path);
	ASSERT(!content.IsEmpty());
	
	int c0 = ann.items.GetCount();
	int c1 = ai_items.GetCount();
	if(!c1) {
		ai_items.SetCount(c0);
		for(int i = 0; i < c0; i++) {
			AnnotationItem& it0 = ann.items[i];
			AiAnnotationItem& it1 = ai_items[i];
			if (it0.begin == it0.end)
				it1.Set(it0, "");
			else {
				String code = GetStringRange(content, it0.begin, it0.end);
				ASSERT(!code.IsEmpty());
				String sha1 = SHA1String(code);
				it1.Set(it0, sha1);
			}
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
					bool match = false;
					if (tries == 0 && it1.IsSameContent(it0)) match = true;
					if (tries == 1 && it0.begin != it0.end) {
						String code = GetStringRange(content, it0.begin, it0.end);
						ASSERT(!code.IsEmpty());
						String sha1 = it0.begin == it0.end ? String() : SHA1String(code);
						auto* range = it1.FindRangeByHashSha1(sha1);
						if (range) {
							// if (range->IsLineAreaPartialMatch(it0)) // <-- not needed anymore
							range->pos = it0.pos;
							range->begin = it0.begin;
							range->end = it0.end;
							match = true;
						}
					}
					if (match) {
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
			if (it0.begin == it0.end)
				it1.Set(it0, "");
			else {
				String code = GetStringRange(content, it0.begin, it0.end);
				ASSERT(!code.IsEmpty());
				String sha1 = SHA1String(code);
				it1.Set(it0, sha1);
			}
			nonlinked0--;
			// it1.linked = &it0;
		}
	}
}

void AiAnnotationItem::SourceRange::RemoveAll(Item::Kind kind) {
	Mutex::Lock ml(lock);
	Vector<int> rmlist;
	for(int i = 0; i < items.GetCount(); i++) {
		if (items[i].kind == kind)
			rmlist << i;
	}
	if (!rmlist.IsEmpty()) items.Remove(rmlist);
}

void AiAnnotationItem::SourceRange::RemoveLineItem(int rel_line)
{
	Mutex::Lock ml(lock);
	Vector<int> rm_list;
	for(int i = 0; i < items.GetCount(); i++) {
		if(items[i].rel_line == rel_line)
			rm_list << i;
	}
	items.Remove(rm_list);
}

AiAnnotationItem::SourceRange::Item* AiAnnotationItem::SourceRange::FindItem(int rel_line)
{
	Mutex::Lock ml(lock);
	for(Item& c : items) {
		if(c.rel_line == rel_line) {
			return &c;
		}
	}
	return 0;
}

String GetStringRange(String content, Point begin, Point end) {
	Vector<String> lines = Split(content, "\n", false);
	{
		int c = lines.GetCount();
		int last_i = c-1;
		if (end.y < last_i) {
			int first_rm = end.y+1;
			int rm_count = c - first_rm;
			lines.Remove(first_rm, rm_count);
		}
		String& last_line = lines.Top();
		if (end.x < last_line.GetCount()) {
			last_line = last_line.Left(end.x);
		}
	}
	if (begin.y > 0) {
		int first_rm = 0;
		int rm_count = min(lines.GetCount(), begin.y);
		if (rm_count > 0)
			lines.Remove(first_rm, rm_count);
		String& first_line = lines[0];
		if (begin.x > 0) {
			first_line = first_line.Mid(begin.x);
		}
	}
	return Join(lines, "\n");
}

END_UPP_NAMESPACE
