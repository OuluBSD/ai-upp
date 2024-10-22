#include <AI/AI.h>
#include <ide/ide.h>

NAMESPACE_UPP

void AionFile::PostSave()
{
	if(!post_saving) {
		post_saving = true;
		PostCallback(THISBACK(Save));
	}
}

void AionFile::Clear() { files.Clear(); }

void AionFile::SetPath(String path)
{
	this->path = path;
	dir = GetFileDirectory(path);
}

void AionFile::Load()
{
	Clear();
	if(FileExists(path)) {
		lock.EnterWrite();
		LoadFromJsonFile(*this, path);
		lock.LeaveWrite();
	}
}

void AionFile::Save()
{
	post_saving = false;
	if(!path.IsEmpty()) {
		lock.EnterWrite();
		StoreAsJsonFile(*this, path);
		lock.LeaveWrite();
	}
}

void AionFile::Jsonize(JsonIO& json) { json("files", files); }

AiFileInfo& AionFile::RealizePath(const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	lock.EnterRead();
	int i = files.Find(rel_file);
	if(i >= 0) {
		AiFileInfo& o = files[i];
		lock.LeaveRead();
		return o;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	AiFileInfo& o = files.Add(rel_file);
	lock.LeaveWrite();
	return o;
}

String AionIndex::ResolveAionFilePath(String path)
{
	Vector<String> parts = Split(GetFileDirectory(path), DIR_SEPS);
	for(int i = 0; i < parts.GetCount(); i++) {
		int c = parts.GetCount() - i;
		if(!c)
			continue;
		String s;
		for(int j = 0; j < c; j++) {
			if(!s.IsEmpty())
				s << DIR_SEPS;
			s << parts[j];
		}
		String upp_path = s + DIR_SEPS + parts[c - 1] + ".upp";
		if(!FileExists(upp_path))
			continue;
		s << DIR_SEPS << "AI.json";
		return s;
	}
	return AppendFileName(GetFileDirectory(path), "AI.json");
}

AionFile& AionIndex::ResolveFile(String path)
{
	String aion_path = ResolveAionFilePath(path);
	lock.EnterRead();
	int i = files.Find(aion_path);
	if(i >= 0) {
		AionFile& f = files[i];
		lock.LeaveRead();
		return f;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	AionFile& f = files.Add(aion_path);
	f.SetPath(aion_path);
	f.Load();
	lock.LeaveWrite();
	return f;
}

AiFileInfo& AionIndex::ResolveFileInfo(String path)
{
	return ResolveFile(path).RealizePath(path);
}

AionIndex& AiIndex() { return Single<AionIndex>(); }

void AionIndex::Load(const String& path, FileAnnotation& fa)
{
	AionFile& af = ResolveFile(path);
	if(af.IsEmpty())
		af.Load();
	AiFileInfo& afi = af.RealizePath(path);
	afi.UpdateLinks(fa);
}

void AionIndex::Store(const String& path, FileAnnotation& fa)
{
	AionFile& af = ResolveFile(path);
	AiFileInfo& afi = af.RealizePath(path);
	afi.UpdateLinks(fa);
	af.Save();
}

END_UPP_NAMESPACE
