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
		LoadFromJsonFile(*this, path);
	}
}

void AionFile::Save()
{
	post_saving = false;
	if(!path.IsEmpty()) {
		// The hash value must be both 32-bit and 64-bit compatible,
		// so use the sha1 hasher instead of the fast hasher
		String current_sha1 = GetHashSha1();
		if (current_sha1 != saved_hash) {
			saved_hash = current_sha1;
			RealizeDirectory(GetFileDirectory(path));
			StoreAsJsonFile(*this, path);
		}
	}
}

String AionFile::GetHashSha1() {
	Sha1Stream s;
	s % *this;
	return s.FinishString();
}

void AionFile::Serialize(Stream& s)
{
	byte version = 1;
	s % version;
	
	if (version >= 1)
		s % saved_hash % files;
}

void AionFile::Jsonize(JsonIO& json)
{
	json("files", files)("saved_hash", saved_hash);
}

AiFileInfo& AionFile::RealizePath(const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	lock.Enter();
	int i = files.Find(rel_file);
	if(i >= 0) {
		AiFileInfo& o = files[i];
		lock.Leave();
		return o;
	}
	AiFileInfo& o = files.Add(rel_file);
	lock.Leave();
	return o;
}

AiFileInfo& AionFile::RealizePath0(const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	int i = files.Find(rel_file);
	if(i >= 0)
		return files[i];
	return files.Add(rel_file);
}

String GetAiPathCandidate(String dir)
{
	Vector<String> ai_dirs = GetAiDirsRaw();
	Vector<String> upp_dirs = GetUppDirs();
	String def_cand, any_ai_cand, preferred_ai_cand;
	def_cand = dir + DIR_SEPS + "AI.json";
	if (!ai_dirs.IsEmpty()) {
		for (const String& upp_dir : upp_dirs) {
			if (dir.Find(upp_dir) != 0) continue;
			String rel_path = dir.Mid(upp_dir.GetCount());
			for (const String& ai_dir : ai_dirs) {
				String ai_dir_cand = AppendFileName(ai_dir, rel_path);
				String path = AppendFileName(ai_dir_cand, "AI.json");
				if (any_ai_cand.IsEmpty())
					any_ai_cand = path;
				if (preferred_ai_cand.IsEmpty() && FileExists(path))
					preferred_ai_cand = path;
			}
		}
	}
	if (!preferred_ai_cand.IsEmpty())
		return preferred_ai_cand;
	else if (!any_ai_cand.IsEmpty())
		return any_ai_cand;
	else
		return def_cand;
}

String AionIndex::ResolveAionFilePath(String path)
{
	String def_dir = GetFileDirectory(path);
	Vector<String> parts = Split(def_dir, DIR_SEPS);
	for(int i = 0; i < parts.GetCount(); i++) {
		int c = parts.GetCount() - i;
		if(!c)
			continue;
		String dir;
		for(int j = 0; j < c; j++) {
			if(!dir.IsEmpty())
				dir << DIR_SEPS;
			dir << parts[j];
		}
		String topname = parts[c - 1];
		String upp_path = dir + DIR_SEPS + topname + ".upp";
		if(!FileExists(upp_path))
			continue;
		return GetAiPathCandidate(dir);
	}
	return GetAiPathCandidate(def_dir);
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
	af.Load(path, fa);
}

void AionFile::Load(const String& path, FileAnnotation& fa)
{
	lock.Enter();
	if(IsEmpty())
		Load();
	AiFileInfo& afi = RealizePath0(path);
	afi.UpdateLinks(fa);
	lock.Leave();
}

void AionIndex::Store(const String& path, FileAnnotation& fa)
{
	AionFile& af = ResolveFile(path);
	af.Store(path, fa);
}

void AionFile::Store(const String& path, FileAnnotation& fa)
{
	AiFileInfo& afi = RealizePath(path);
	lock.Enter();
	afi.UpdateLinks(fa);
	Save();
	lock.Leave();
}

END_UPP_NAMESPACE
